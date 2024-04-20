#include "mmu_table.hpp"

#include "mmu_defs.hpp"

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

static inline constexpr uintptr_t TTBR_MASK = 0xffff000000000000;

static inline constexpr uintptr_t TABLE_ENTRIES = PAGE_SIZE / sizeof(uint64_t);

static inline constexpr uint64_t PAGE_MARKER = 0b11;
static inline constexpr uint64_t BLOCK_MARKER = 0b01;
static inline constexpr uint64_t TABLE_MARKER = 0b11;

enum class EntryKind { Invalid, Table, Page, Block };

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

uint64_t encode_new_entry(MMUTable* tbl, PhysicalPA pa, PagesAttributes attr, PageSize amount) {
  const uint64_t upper_attr = (uint64_t)attr.exec << 53;

  const uint64_t lower_attr = ((uint64_t)attr.type << 2) | ((uint64_t)attr.access << 6) | ((uint64_t)attr.rw << 7) |
                              ((uint64_t)attr.sh << 8) |
                              ((uint64_t)1 << 10)  // Set the Access flag to disable access interrupts
                              | ((uint64_t)(tbl->kind == MMUTable::Kind::Process ? 1 : 0) << 11);

  const uint64_t marker = amount == PageSize::Page_4Kio ? PAGE_MARKER : BLOCK_MARKER;

  const uint64_t new_entry = upper_attr | (pa & libk::mask_bits(12, 47)) | lower_attr | marker;

  return new_entry;
}

static inline constexpr uint64_t* get_table_ptr_from_entry(MMUTable* tbl, uint64_t entry) {
  if (tbl->resolve_pa == nullptr) {
    return nullptr;
  }

  VirtualPA sub_table_va;
  if (!tbl->resolve_pa(tbl->handle, get_table_pa_from_entry(entry), &sub_table_va)) {
    // Unable to convert the physical address back to a virtual one :/
    return nullptr;
  }

  return (uint64_t*)sub_table_va;
}

inline void invalid_va(MMUTable* table, VirtualPA entry_va) {
  const uint64_t page_index = (entry_va & libk::mask_bits(12, 47)) >> 12;

  switch (table->kind) {
    case MMUTable::Kind::Kernel: {
      // Encoding from page D8-6757
      asm volatile("tlbi vaae1, %x0" ::"r"(page_index));
      break;
    }
    case MMUTable::Kind::Process: {
      // Encoding from page D8-6757
      const uint64_t tmp = ((uint64_t)table->asid << 48) | page_index;
      asm volatile("tlbi vae1, %x0" ::"r"(tmp));
      break;
    }
  }
}

uint64_t* find_table_for_entry(MMUTable* tbl, VirtualPA entry_va, size_t entry_level, bool create_table_if_missing) {
  if (tbl->resolve_va == nullptr || tbl->alloc == nullptr) {
    return nullptr;
  }

  if (tbl->kind == MMUTable::Kind::Kernel && (entry_va & TTBR_MASK) != KERNEL_BASE) {
    // Invalid kernel address
    return nullptr;
  };

  if (tbl->kind == MMUTable::Kind::Process && (entry_va & TTBR_MASK) != PROCESS_BASE) {
    // Invalid process address
    return nullptr;
  }

  // Check that the virtual address is correctly aligned with the specified amount mapped.
  if ((entry_va & libk::mask_bits(0, 12 + 9 * (4 - entry_level) - 1)) != 0) {
    return nullptr;
  }

  size_t current_level = 1;
  auto* current_table = (uint64_t*)tbl->pgd;

  while (current_level < entry_level) {
    uint64_t current_table_index = get_index_in_table(entry_va, current_level);
    const uint64_t entry = current_table[current_table_index];

    switch (get_entry_kind(entry, current_level)) {
      case EntryKind::Invalid: {
        if (!create_table_if_missing) {
          // Missing table and we can't create one :/
          return nullptr;
        }

        // We need a new page, let's allocate it !
        VirtualPA new_table;
        if (!tbl->alloc(tbl->handle, &new_table)) {
          // Unable to allocate a fresh table :/
          return nullptr;
        }

        // We resolve its physical address
        PhysicalPA new_table_pa;
        if (!tbl->resolve_va(tbl->handle, new_table, &new_table_pa)) {
          return nullptr;
        }

        // And fill the current table with the new entry
        current_table[current_table_index] = new_table_pa | TABLE_MARKER;

        // We update the current table now
        current_table = (uint64_t*)new_table;
        break;
      }
      case EntryKind::Table: {
        current_table = get_table_ptr_from_entry(tbl, entry);

        if (current_table == nullptr) {
          return nullptr;
        }
        break;
      }
      case EntryKind::Page:
      case EntryKind::Block:
        /* Expected a Table, got a mapping :/ */
        return nullptr;
    }

    ++current_level;
  }

  return current_table;
}

bool map_chunk(MMUTable* tbl, VirtualPA va, PhysicalPA pa, PageSize amount, PagesAttributes attr) {
  const auto entry_level = (size_t)amount;
  uint64_t* table_for_entry = find_table_for_entry(tbl, va, entry_level, true);

  if (table_for_entry == nullptr) {
    return false;
  }

  const size_t entry_index = get_index_in_table(va, entry_level);
  const uint64_t existing_entry = table_for_entry[entry_index];

  switch (get_entry_kind(existing_entry, entry_level)) {
    case EntryKind::Table: {
      // Cannot replace an existing table with a block entry :/
      return false;
    }
    case EntryKind::Page:
    case EntryKind::Block: {
      const auto existing_pa = get_table_pa_from_entry(existing_entry);
      return existing_pa == pa;
    }
    case EntryKind::Invalid: {
      // Good we can work !
      break;
    }
  }

  const uint64_t new_entry = encode_new_entry(tbl, pa, attr, amount);
  table_for_entry[entry_index] = new_entry;

  return true;
}

bool unmap_chunk(MMUTable* tbl, VirtualPA va, PageSize amount) {
  const auto table_level = (size_t)amount;
  uint64_t* table = find_table_for_entry(tbl, va, table_level, false);

  if (table == nullptr) {
    return false;
  }

  const size_t entry_index = get_index_in_table(va, table_level);
  const uint64_t existing_entry = table[entry_index];

  switch (get_entry_kind(existing_entry, table_level)) {
    case EntryKind::Table: {
      // Cannot unmap a whole table :/
      return false;
    }
    case EntryKind::Invalid: {
      // Nothing to do !
      return true;
    }
    case EntryKind::Page:
    case EntryKind::Block: {
      // Good we can work !
      table[entry_index] = 0ull;

      invalid_va(tbl, va);
      return true;
    }
  }

  return true;  // Why GCC ?!
}

bool has_entry_at(MMUTable* tbl, VirtualPA va, PageSize amount) {
  const auto entry_level = (size_t)amount;
  return find_table_for_entry(tbl, va, entry_level, false) != nullptr;
}

/** All bound are *INCLUSIVE* */
bool map_range_in_table(MMUTable* tbl,
                        VirtualPA va_start,
                        VirtualPA va_end,
                        PhysicalPA pa_start,
                        PagesAttributes attr,
                        uint64_t* table,
                        size_t table_level,
                        VirtualPA table_first_page_va) {
  if (tbl == nullptr || table == nullptr || tbl->alloc == nullptr || tbl->resolve_va == nullptr) {
    return false;
  }

  if (va_start > va_end) {
    return true;
  }

  const auto table_last_page_va = table_last_page(table_first_page_va, table_level);

  if ((va_end < table_first_page_va) || (va_start > table_last_page_va)) {
    // Input address space does not intersect with this table.
    return true;
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
        if (table_level == 4) {
          // Map with a page
          const size_t offset = entry_va_start - va_start;
          const auto entry_pa = pa_start + offset;
          const uint64_t new_entry = encode_new_entry(tbl, entry_pa, attr, PageSize::Page_4Kio);
          table[index] = new_entry;
        } else {
          // table_level < 4
          const auto entry_va_end = table_last_page(entry_va_start, table_level + 1);

          if (va_start <= entry_va_start && entry_va_end <= va_end && table_level >= 2) {
            // Map with a block
            const size_t offset = entry_va_start - va_start;
            const auto entry_pa = pa_start + offset;
            const uint64_t new_entry = encode_new_entry(tbl, entry_pa, attr, (PageSize)table_level);
            table[index] = new_entry;
          } else {
            // Map with a table

            // We need a new page, let's allocate it !
            VirtualPA new_table;
            if (!tbl->alloc(tbl->handle, &new_table)) {
              // Unable to allocate a fresh table :/
              return false;
            }

            // We resolve its physical address
            PhysicalPA new_table_pa;
            if (!tbl->resolve_va(tbl->handle, new_table, &new_table_pa)) {
              return false;
            }

            // And fill the current table with the new entry
            table[index] = new_table_pa | TABLE_MARKER;

            // And recursively map what's needed.
            if (!map_range_in_table(tbl, va_start, va_end, pa_start, attr, (uint64_t*)new_table, table_level + 1,
                                    entry_va_start)) {
              return false;
            }
          }
        }
        break;
      }

      case EntryKind::Block:
      case EntryKind::Page: {
        const size_t offset = entry_va_start - va_start;

        if (pa_start + offset != get_table_pa_from_entry(entry)) {
          // Incompatible intersection found :/
          return false;
        } else {
          continue;
        }
      }

      case EntryKind::Table: {
        uint64_t* sub_table = get_table_ptr_from_entry(tbl, entry);

        // And recursively map what's needed.
        if (!map_range_in_table(tbl, va_start, va_end, pa_start, attr, sub_table, table_level + 1, entry_va_start)) {
          return false;
        }
      }
    }
  }

  return true;
}

/** All bound are *INCLUSIVE* */
bool map_range(MMUTable* tbl, VirtualPA va_start, VirtualPA va_end, PhysicalPA pa_start, PagesAttributes attr) {
  if (tbl == nullptr) {
    return false;
  }

  if (va_start > va_end) {
    return true;
  }

  if (va_start == va_end) {
    return map_chunk(tbl, va_start, pa_start, PageSize::Page_4Kio, attr);
  }

  const auto initial_va = VirtualPA(tbl->kind == MMUTable::Kind::Kernel ? KERNEL_BASE : PROCESS_BASE);

  return map_range_in_table(tbl, va_start, va_end, pa_start, attr, (uint64_t*)tbl->pgd, 1, initial_va);
}

void clear_table(MMUTable* tbl, uint64_t* table, size_t table_level, VirtualPA table_va) {
  if (tbl == nullptr || tbl->free == nullptr || table == nullptr) {
    return;
  }

  for (size_t i = 0; i < TABLE_ENTRIES; ++i) {
    const uint64_t entry = table[i];
    const VirtualPA entry_va = get_entry_va_from_table_index(table_va, table_level, i);
    table[i] = 0ull;  // Clear entry

    switch (get_entry_kind(entry, table_level)) {
      case EntryKind::Invalid: {
        // Nothing else to do good !
        break;
      }
      case EntryKind::Table: {
        uint64_t* sub_table = get_table_ptr_from_entry(tbl, entry);

        if (sub_table != nullptr) {
          clear_table(tbl, sub_table, table_level + 1, entry_va);
          tbl->free(tbl->handle, VirtualPA((uintptr_t)sub_table));
        }
        break;
      }

      case EntryKind::Page:
      case EntryKind::Block: {
        invalid_va(tbl, entry_va);
        break;
      }
    }
  }
}

void clear_all(MMUTable* tbl) {
  if (tbl == nullptr) {
    return;
  }

  const auto initial_va = VirtualPA(tbl->kind == MMUTable::Kind::Kernel ? KERNEL_BASE : PROCESS_BASE);

  clear_table(tbl, (uint64_t*)tbl->pgd, 1, initial_va);
}

bool change_properties_for_range(VirtualPA va_start,
                                 VirtualPA va_end,
                                 PagesAttributes attr,
                                 uint64_t* table,
                                 size_t table_level,
                                 VirtualPA table_first_page_va) {
  if (va_start > va_end) {
    return true;
  }

  const auto table_last_page_va = table_last_page(table_first_page_va, table_level);

  if ((va_end < table_first_page_va) || (va_start > table_last_page_va)) {
    // Input address space does not intersect with this table.
    return true;
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
        // Nothing to change here !
        continue;
      }

      case EntryKind::Block: {
        // If split needed:
        // - Decode Attrs and PA
        // - We split in 3 parts : BeforeChange, Change, AfterChange
        // - We call map_range for each part
        break;
      }
      case EntryKind::Page: {
        // Trivial case in theory
        break;
      }

      case EntryKind::Table: {
        PhysicalPA sub_table_pa = get_table_pa_from_entry(entry);

        // HERE, and ONLY HERE, VirtualAddress = PhysicalAddress
        auto* sub_table = (uint64_t*)sub_table_pa;

        // And recursively map what's needed.
        if (!change_properties_for_range(va_start, va_end, attr, sub_table, table_level + 1, entry_va_start)) {
          return false;
        }
      }
    }
  }

  return false;
}
