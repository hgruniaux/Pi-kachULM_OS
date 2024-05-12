#include "mmu_table.hpp"
#include <libk/utils.hpp>
#include "boot/mmu_utils.hpp"

/*
 63     48             38       30             20       12                                          Physical Memory
+---------+-----------+-----------+-----------+-----------+-------------+                         +------------------+
|         | PGD Index | PUD Index | PMD Index | PTE Index | Page offset |                         |                  |
+---------+-----------+-----------+-----------+-----------+-------------+                         |                  |
           47   |   39        |    29    |  21      |      11     |    0                          |                  |
                |             |          |          |             |                               |      Page N      |
          +-----+             |          |          |             +----------------+          +-->+------------------+
+------+  |        PGD        |          |          +-------------------+          |          |   |                  |
| ttbr |---->+-------------+  |          +---------+                    |          |          |   |                  |
+------+  |  |             |  |                    |                    |          |          |   +------------------+
          |  +-------------+  |         PUD        |                    |          +----------|-->| Physical address |
          +->| PUD address |----->+-------------+  |                    |                     |   +------------------+
             +-------------+  |   |             |  |                    |                     |   |                  |
             |             |  |   +-------------+  |         PMD        |                     |   |                  |
             +-------------+  +-->| PMD address |----->+-------------+  |                     |   +------------------+
                    ^             +-------------+  |   |             |  |                     |   |                  |
                  Level 1         |             |  |   +-------------+  |         PTE         |   |                  |
                                  +-------------+  +-->| PTE address |----->+--------------+  |   |                  |
                                         ^             +-------------+  |   |              |  |   |                  |
                                       Level 2         |             |  |   +--------------+  |   |                  |
                                                       +-------------+  +-->| Page address |--+   |                  |
                                                              ^             +--------------+      |                  |
                                                            Level 3         |              |      |                  |
                                                                            +--------------+      +------------------+
                                                                                    ^
                                                                                  Level 4
 */

static inline constexpr uintptr_t PAGE_ADDRESS_MASK = libk::mask_bits(0, 11);

static inline constexpr uintptr_t TABLE_ENTRIES = PAGE_SIZE / sizeof(uint64_t);

static inline constexpr uint64_t PAGE_MARKER = 0b11;
static inline constexpr uint64_t BLOCK_MARKER = 0b01;
static inline constexpr uint64_t TABLE_MARKER = 0b11;

enum class EntryKind { Invalid, Table, Page, Block };

static inline void data_sync() {
  asm volatile("dsb sy");
}

static inline uint64_t get_base_address(const MMUTable* tbl) {
  switch (tbl->kind) {
    case MMUTable::Kind::Kernel:
      return KERNEL_BASE;
    case MMUTable::Kind::Process:
      return PROCESS_BASE;
  }

  return -1;
}

static inline bool check_va(const MMUTable* tbl, VirtualPA va) {
  if ((va & TTBR_MASK) != get_base_address(tbl)) {
    // Invalid kernel address
    return false;
  };

  if ((va & PAGE_ADDRESS_MASK) != 0) {
    return false;
  }

  return true;
}

static inline constexpr EntryKind get_entry_kind(uint64_t entry, size_t level) {
  if ((entry & 0b1) == 0) {
    // Entry last bits are 0bx1
    return EntryKind::Invalid;
  } else if ((entry & 0b10) == 0) {
    // Entry last bits are 0b01
    return EntryKind::Block;
  } else if (level == 4) {
    // Entry last bits are 0b11
    return EntryKind::Page;
  } else {
    // Entry last bits are 0b11
    return EntryKind::Table;
  }
}

static inline constexpr PhysicalPA get_table_pa_from_entry(uint64_t entry) {
  return PhysicalPA(entry & libk::mask_bits(12, 47));
}

static inline constexpr uint64_t get_index_in_table(VirtualPA va, size_t level) {
  const size_t index_shift = 12 + 9 * (4 - level);
  // We mask the 9 first bits.
  return (va >> index_shift) & libk::mask_bits(0, 8);
}

static inline constexpr VirtualPA get_entry_va_from_table_index(VirtualPA table_va,
                                                                size_t table_level,
                                                                size_t entry_index) {
  return table_va + (entry_index << (12 + 9 * (4 - table_level)));
}

static inline constexpr VirtualPA table_last_page(VirtualPA table_first_page_va, size_t table_level) {
  // Check that the virtual address is correctly aligned with the specified level.
  KASSERT((table_first_page_va & libk::mask_bits(0, 12 + 9 * (4 - table_level) - 1)) == 0);

  return table_first_page_va + libk::mask_bits(12, 12 + 9 * (4 - table_level + 1) - 1);
}

uint64_t encode_new_entry(MMUTable* tbl, PhysicalPA pa, size_t entry_level, PagesAttributes attr) {
  const uint64_t upper_attr = (uint64_t)attr.exec << 53;

  const uint8_t nG_flag = tbl->kind == MMUTable::Kind::Kernel ? 0 : 1;

  const uint64_t lower_attr = ((uint64_t)attr.type << 2) | ((uint64_t)attr.access << 6) | ((uint64_t)attr.rw << 7) |
                              ((uint64_t)attr.sh << 8) |
                              ((uint64_t)1 << 10)  // Set the Access flag to disable access interrupts
                              | (nG_flag << 11);

  const uint64_t marker = entry_level == 4 ? PAGE_MARKER : BLOCK_MARKER;

  const uint64_t new_entry = upper_attr | (pa & libk::mask_bits(12, 47)) | lower_attr | marker;

  return new_entry;
}

static inline constexpr PhysicalPA decode_entry(const uint64_t entry, PagesAttributes* attr) {
  if (attr != nullptr) {
    attr->exec = (ExecutionPermission)((entry >> 53) & 0b11);
    attr->sh = (Shareability)((entry >> 8) & 0b11);
    attr->rw = (ReadWritePermission)((entry >> 7) & 0b1);
    attr->access = (Accessibility)((entry >> 6) & 0b1);
    attr->type = (MemoryType)((entry >> 2) & 0b11);
  }

  return entry & libk::mask_bits(12, 47);
}

void reload_tlb(const MMUTable* table) {
  switch (table->kind) {
    case MMUTable::Kind::Kernel: {
      // Encoding from page D8-6757
      asm volatile("tlbi vmalle1is");
      break;
    }
    case MMUTable::Kind::Process: {
      // Encoding from page D8-6757
      const uint64_t tmp = ((uint64_t)table->asid << 48);
      asm volatile("tlbi aside1, %x0" ::"r"(tmp));
      break;
    }
  }
  asm volatile("dsb sy; isb");
}

// Prerequisites :
// - tbl != nullptr
// - cur_tbl != nullptr
// - tbl->resolve_pa != nullptr
// - check_va(tbl, entry_va)
[[nodiscard]] bool find_entry_in_table(const MMUTable* tbl,
                                       uint64_t* cur_tbl,
                                       size_t cur_lvl,
                                       VirtualPA entry_va,
                                       uint64_t** found_table,
                                       size_t* found_index,
                                       size_t* found_level) {
  const size_t cur_index = get_index_in_table(entry_va, cur_lvl);
  const uint64_t cur_entry = cur_tbl[cur_index];
  switch (get_entry_kind(cur_entry, cur_lvl)) {
    case EntryKind::Invalid: {
      return false;
    }
    case EntryKind::Table: {
      auto sub_table_ptr = (uint64_t*)tbl->resolve_pa(tbl->handle, get_table_pa_from_entry(cur_entry));
      if (sub_table_ptr == nullptr) {
        return false;
      }
      return find_entry_in_table(tbl, sub_table_ptr, cur_lvl + 1, entry_va, found_table, found_index, found_level);
    }
    case EntryKind::Page:
      [[fallthrough]];
    case EntryKind::Block: {
      if (found_table != nullptr) {
        *found_table = cur_tbl;
      }

      if (found_index != nullptr) {
        *found_index = cur_index;
      }

      if (found_level != nullptr) {
        *found_level = cur_lvl;
      }

      return true;
    }
  }

  return false;
}

bool get_attr(const MMUTable* tbl, VirtualPA va, PagesAttributes* attr) {
  if (tbl == nullptr || (uint64_t*)tbl->pgd == nullptr || tbl->resolve_pa == nullptr || !check_va(tbl, va)) {
    return false;
  }

  uint64_t* va_table;
  size_t va_index;

  if (!find_entry_in_table(tbl, (uint64_t*)tbl->pgd, 1, va, &va_table, &va_index, nullptr)) {
    // No entry found :/
    return false;
  }

  (void)decode_entry(va_table[va_index], attr);

  return true;
}

bool change_attr_va(MMUTable* tbl, VirtualPA va, PagesAttributes attr) {
  if (tbl == nullptr || (uint64_t*)tbl->pgd == nullptr || tbl->resolve_pa == nullptr || !check_va(tbl, va)) {
    return false;
  }

  uint64_t* va_table;
  size_t va_index;
  size_t va_level;

  if (!find_entry_in_table(tbl, (uint64_t*)tbl->pgd, 1, va, &va_table, &va_index, &va_level)) {
    // No entry found :/
    return false;
  }

  // Entry found !
  const uint64_t old_pa = decode_entry(va_table[va_index], nullptr);
  const uint64_t new_entry = encode_new_entry(tbl, old_pa, va_level, attr);
  va_table[va_index] = 0ull;
  data_sync();
  reload_tlb(tbl);
  va_table[va_index] = new_entry;
  data_sync();
  return true;
}

/** All bound are *INCLUSIVE*
 * Prerequisites :
 *  - tbl != nullptr
 *  - tbl->alloc != nullptr
 *  - tbl->resolve_va != nullptr
 *  - tbl->resolve_pa != nullptr
 *  - check_va(tbl, va_start)
 *  - check_va(tbl, va_end)
 *  - table != nullptr
 */
void map_range_in_table(MMUTable* tbl,
                        VirtualPA va_start,
                        VirtualPA va_end,
                        PhysicalPA pa_start,
                        PagesAttributes attr,
                        uint64_t* table,
                        size_t table_level,
                        VirtualPA table_first_page_va) {
  const auto table_last_page_va = table_last_page(table_first_page_va, table_level);

  if ((va_end < table_first_page_va) || (va_start > table_last_page_va) || (va_start > va_end)) {
    // Input address space does not intersect with this table.
    return;
  }

  const auto inter_start_va = libk::max(va_start, table_first_page_va);
  const auto inter_end_va = libk::min(va_end, table_last_page_va);

  // We need to check all entries of the table within [start_table_index:end_table_index].
  const size_t inter_start_index = get_index_in_table(inter_start_va, table_level);
  const size_t inter_end_index = get_index_in_table(inter_end_va, table_level);

  for (size_t index = inter_start_index; index <= inter_end_index; ++index) {
    const uint64_t entry = table[index];
    const auto entry_va_start = get_entry_va_from_table_index(table_first_page_va, table_level, index);

    const auto entry_kind = get_entry_kind(entry, table_level);

    if (entry_kind == EntryKind::Table) {
      auto* sub_table = (uint64_t*)tbl->resolve_pa(tbl->handle, get_table_pa_from_entry(entry));
      map_range_in_table(tbl, va_start, va_end, pa_start, attr, sub_table, table_level + 1, entry_va_start);
      continue;
    }

    // entry_kind = Block, Page or Invalid
    const auto entry_va_stop = table_last_page(entry_va_start, table_level + 1);

    if (table_level == 4 || (va_start <= entry_va_start && entry_va_stop <= va_end && table_level >= 2)) {
      // Map with a block/table
      const size_t offset = entry_va_start - va_start;
      const auto entry_pa = pa_start + offset;
      const uint64_t new_entry = encode_new_entry(tbl, entry_pa, table_level, attr);
      if (entry_kind != EntryKind::Invalid) {
        // entry_kind = Block or Page -> Need to invalidate previous entry
        table[index] = 0ull;
        data_sync();
        reload_tlb(tbl);
      }

      table[index] = new_entry;
      data_sync();
      continue;
    }

    // table_level < 4
    // entry_kind = Block or Invalid
    // Map with a table

    // Store old attributes in case it was a block
    PagesAttributes old_attr = {};
    PhysicalPA old_pa = decode_entry(entry, &old_attr);

    if (entry_kind == EntryKind::Block) {
      // entry_kind = Block -> Need to invalidate previous entry
      table[index] = 0ull;
      data_sync();
      reload_tlb(tbl);
    }

    VirtualPA new_table = tbl->alloc(tbl->handle);
    PhysicalPA new_table_pa = tbl->resolve_va(tbl->handle, new_table);
    table[index] = new_table_pa | TABLE_MARKER;
    data_sync();

    map_range_in_table(tbl, va_start, va_end, pa_start, attr, (uint64_t*)new_table, table_level + 1, entry_va_start);

    if (entry_kind == EntryKind::Block) {
      // Remap untouched regions
      map_range_in_table(tbl, entry_va_start, va_start - PAGE_SIZE, old_pa, old_attr, (uint64_t*)new_table,
                         table_level + 1, entry_va_start);

      // Mapping ]VA_End; Entry_VA_End] -> [Entry_PA + PA_Offset; ...[
      size_t pa_offset = va_end + PAGE_SIZE - entry_va_start;
      map_range_in_table(tbl, va_end + PAGE_SIZE, entry_va_stop, old_pa + pa_offset, old_attr, (uint64_t*)new_table,
                         table_level + 1, entry_va_start);
    }
  }
}

/** All bound are *INCLUSIVE* */
bool map_range(MMUTable* tbl, VirtualPA va_start, VirtualPA va_end, PhysicalPA pa_start, PagesAttributes attr) {
  if (tbl == nullptr || tbl->alloc == nullptr || tbl->resolve_va == nullptr || tbl->resolve_pa == nullptr ||
      (uint64_t*)tbl->pgd == nullptr || !check_va(tbl, va_start) || !check_va(tbl, va_end)) {
    return false;
  }

  map_range_in_table(tbl, va_start, va_end, pa_start, attr, (uint64_t*)tbl->pgd, 1, get_base_address(tbl));
  return true;
}

/** All bound are *INCLUSIVE*
 * Prerequisites :
 *  - tbl != nullptr
 *  - tbl->alloc != nullptr
 *  - tbl->free != nullptr
 *  - tbl->resolve_va != nullptr
 *  - tbl->resolve_pa != nullptr
 *  - check_va(tbl, va_start)
 *  - check_va(tbl, va_end)
 *  - table != nullptr
 */
void unmap_range_in_table(MMUTable* tbl,
                          VirtualPA va_start,
                          VirtualPA va_end,
                          uint64_t* table,
                          size_t table_level,
                          VirtualPA table_first_page_va) {
  const auto table_last_page_va = table_last_page(table_first_page_va, table_level);

  if ((va_end < table_first_page_va) || (va_start > table_last_page_va) || (va_start > va_end)) {
    // Input address space does not intersect with this table.
    return;
  }

  const auto inter_start_va = libk::max(va_start, table_first_page_va);
  const auto inter_end_va = libk::min(va_end, table_last_page_va);

  // We need to check all entries of the table within [start_table_index:end_table_index].
  const size_t inter_start_index = get_index_in_table(inter_start_va, table_level);
  const size_t inter_end_index = get_index_in_table(inter_end_va, table_level);

  for (size_t index = inter_start_index; index <= inter_end_index; ++index) {
    const uint64_t entry = table[index];
    const auto entry_va_start = get_entry_va_from_table_index(table_first_page_va, table_level, index);

    switch (get_entry_kind(entry, table_level)) {
      case EntryKind::Invalid: {
        // Nothing to do here !
        break;
      }

      case EntryKind::Block: {
        // May need to split the block :/
        const auto entry_va_stop = table_last_page(table_first_page_va, table_level + 1);

        if (va_start <= entry_va_start && entry_va_stop <= va_end) {
          // Can erase the whole block !
          table[index] = 0ull;
          data_sync();
          reload_tlb(tbl);
        } else {
          // Need to split :/
          // 1. Retrieve attributes and page
          PagesAttributes entry_attr{};
          PhysicalPA entry_pa = decode_entry(entry, &entry_attr);

          // 2. Allocate new table
          VirtualPA new_table = tbl->alloc(tbl->handle);
          PhysicalPA new_table_pa = tbl->resolve_va(tbl->handle, new_table);
          table[index] = 0ull;
          data_sync();
          reload_tlb(tbl);
          table[index] = new_table_pa | TABLE_MARKER;
          data_sync();

          // 3. And map untouched regions

          // Mapping [Entry_VA_Start; VA_Start[ -> [Entry_PA; ...[
          map_range_in_table(tbl, entry_va_start, va_start - PAGE_SIZE, entry_pa, entry_attr, (uint64_t*)new_table,
                             table_level + 1, entry_va_start);

          // Mapping ]VA_End; Entry_VA_End] -> [Entry_PA + PA_Offset; ...[
          size_t pa_offset = va_end + PAGE_SIZE - entry_va_start;
          map_range_in_table(tbl, va_end + PAGE_SIZE, entry_va_stop, entry_pa + pa_offset, entry_attr,
                             (uint64_t*)new_table, table_level + 1, entry_va_start);
        }
        break;
      }

      case EntryKind::Page: {
        table[index] = 0ull;
        data_sync();
        reload_tlb(tbl);
        break;
      }

      case EntryKind::Table: {
        const auto entry_va_stop = table_last_page(table_first_page_va, table_level + 1);
        VirtualPA sub_table_va = tbl->resolve_pa(tbl->handle, get_table_pa_from_entry(entry));

        unmap_range_in_table(tbl, va_start, va_end, (uint64_t*)sub_table_va, table_level + 1, entry_va_start);

        if (va_start <= entry_va_start && entry_va_stop <= va_end) {
          // The whole page as been unmapped, we can free it !
          tbl->free(tbl->handle, sub_table_va);
          table[index] = 0ull;
          data_sync();
        }
      }
    }
  }
}

/** All bound are *INCLUSIVE* */
bool unmap_range(MMUTable* tbl, VirtualPA va_start, VirtualPA va_end) {
  if (tbl == nullptr || tbl->alloc == nullptr || tbl->free == nullptr || tbl->resolve_va == nullptr ||
      tbl->resolve_pa == nullptr || (uint64_t*)tbl->pgd == nullptr || !check_va(tbl, va_start) ||
      !check_va(tbl, va_end)) {
    return false;
  }

  unmap_range_in_table(tbl, va_start, va_end, (uint64_t*)tbl->pgd, 1, get_base_address(tbl));
  return true;
}

/**
 * Prerequisites :
 *  - tbl != nullptr
 *  - tbl->free != nullptr
 *  - tbl->resolve_pa != nullptr
 *  - table != nullptr
 */
void clear_table(MMUTable* tbl, uint64_t* table, size_t table_level, VirtualPA table_va) {
  for (size_t i = 0; i < TABLE_ENTRIES; ++i) {
    const uint64_t entry = table[i];
    const VirtualPA entry_va = get_entry_va_from_table_index(table_va, table_level, i);
    table[i] = 0ull;  // Clear entry
    data_sync();

    switch (get_entry_kind(entry, table_level)) {
      case EntryKind::Invalid: {
        // Nothing else to do good !
        break;
      }
      case EntryKind::Table: {
        auto* sub_table = (uint64_t*)tbl->resolve_pa(tbl->handle, get_table_pa_from_entry(entry));
        clear_table(tbl, sub_table, table_level + 1, entry_va);
        tbl->free(tbl->handle, VirtualPA((uintptr_t)sub_table));
        break;
      }

      case EntryKind::Page:
      case EntryKind::Block: {
        reload_tlb(tbl);
        break;
      }
    }
  }
}

void clear_all(MMUTable* tbl) {
  if (tbl == nullptr || tbl->free == nullptr || tbl->resolve_pa == nullptr || (uint64_t*)tbl->pgd == nullptr) {
    return;
  }

  clear_table(tbl, (uint64_t*)tbl->pgd, 1, get_base_address(tbl));
}

/** All bound are *INCLUSIVE*
 * Prerequisites :
 *  - tbl != nullptr
 *  - tbl->alloc != nullptr
 *  - tbl->resolve_va != nullptr
 *  - tbl->resolve_pa != nullptr
 *  - check_va(tbl, va_start)
 *  - check_va(tbl, va_end)
 *  - table != nullptr
 */
void change_properties_for_range(MMUTable* tbl,
                                 VirtualPA va_start,
                                 VirtualPA va_end,
                                 PagesAttributes attr,
                                 uint64_t* table,
                                 size_t table_level,
                                 VirtualPA table_first_page_va) {
  const auto table_last_page_va = table_last_page(table_first_page_va, table_level);

  if ((va_end < table_first_page_va) || (va_start > table_last_page_va) || (va_start > va_end)) {
    // Input address space does not intersect with this table.
    return;
  }

  const auto inter_start_va = libk::max(va_start, table_first_page_va);
  const auto inter_end_va = libk::min(va_end, table_last_page_va);

  // We need to check all entries of the table within [start_table_index:end_table_index].
  const size_t inter_start_index = get_index_in_table(inter_start_va, table_level);
  const size_t inter_end_index = get_index_in_table(inter_end_va, table_level);

  for (size_t index = inter_start_index; index <= inter_end_index; ++index) {
    const uint64_t entry = table[index];
    const auto entry_va_start = get_entry_va_from_table_index(table_first_page_va, table_level, index);

    switch (get_entry_kind(entry, table_level)) {
      case EntryKind::Invalid: {
        // Nothing to do here !
        break;
      }

      case EntryKind::Block: {
        // May need to split the block :/
        const auto entry_va_stop = table_last_page(table_first_page_va, table_level + 1);

        PagesAttributes entry_attr{};
        PhysicalPA entry_pa = decode_entry(entry, &entry_attr);

        if (va_start <= entry_va_start && entry_va_stop <= va_end) {
          // We can change the whole block !
          table[index] = encode_new_entry(tbl, entry_pa, table_level, attr);
          data_sync();
          reload_tlb(tbl);
        } else {
          // Need to split :/
          // 1. Allocate new table
          VirtualPA new_table = tbl->alloc(tbl->handle);
          PhysicalPA new_table_pa = tbl->resolve_va(tbl->handle, new_table);
          table[index] = new_table_pa | TABLE_MARKER;
          data_sync();
          reload_tlb(tbl);

          // 2. And map untouched regions

          // Mapping [Entry_VA_Start; VA_Start[ -> [Entry_PA; ...[
          map_range_in_table(tbl, entry_va_start, va_start - PAGE_SIZE, entry_pa, entry_attr, (uint64_t*)new_table,
                             table_level + 1, entry_va_start);

          // Mapping [VA_Start; VA_End] -> [Entry_PA + PA_Offset1; ...[
          size_t pa_offset1 = va_start - entry_va_start;
          map_range_in_table(tbl, va_start, va_end, entry_pa + pa_offset1, attr, (uint64_t*)new_table, table_level + 1,
                             entry_va_start);

          // Mapping ]VA_End; Entry_VA_End] -> [Entry_PA + PA_Offset2; ...[
          size_t pa_offset2 = va_end + PAGE_SIZE - entry_va_start;
          map_range_in_table(tbl, va_end + PAGE_SIZE, entry_va_stop, entry_pa + pa_offset2, entry_attr,
                             (uint64_t*)new_table, table_level + 1, entry_va_start);
        }

        break;
      }

      case EntryKind::Page: {
        PhysicalPA entry_pa = decode_entry(entry, nullptr);

        table[index] = encode_new_entry(tbl, entry_pa, table_level, attr);
        data_sync();
        reload_tlb(tbl);
        break;
      }

      case EntryKind::Table: {
        auto* sub_table = (uint64_t*)tbl->resolve_pa(tbl->handle, get_table_pa_from_entry(entry));
        change_properties_for_range(tbl, va_start, va_end, attr, sub_table, table_level + 1, entry_va_start);
        break;
      }
    }
  }
}

bool change_attr_range(MMUTable* tbl, VirtualPA va_start, VirtualPA va_end, PagesAttributes attr) {
  if (tbl == nullptr || tbl->alloc == nullptr || tbl->resolve_va == nullptr || tbl->resolve_pa == nullptr ||
      (uint64_t*)tbl->pgd == nullptr || !check_va(tbl, va_start) || !check_va(tbl, va_end)) {
    return false;
  }

  change_properties_for_range(tbl, va_start, va_end, attr, (uint64_t*)tbl->pgd, 1, get_base_address(tbl));
  return true;
}
