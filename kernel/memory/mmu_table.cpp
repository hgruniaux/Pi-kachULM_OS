#include <cassert>

#include "mmu.h"
#include "mmu_table.hpp"

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

enum class EntryKind { Invalid, Table, Page, Block };

static inline constexpr uint64_t PAGE_MARKER = 0b11;
static inline constexpr uint64_t BLOCK_MARKER = 0b01;
static inline constexpr uint64_t TABLE_MARKER = 0b11;

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

static inline constexpr libk::PhysicalPA get_table_pa_from_entry(uint64_t entry) {
  return libk::PhysicalPA(entry & libk::mask_bits(12, 47));
}

static inline constexpr uint64_t* get_table_ptr_from_entry(uint64_t entry, libk::PageMapper* mapper) {
  if (mapper == nullptr) {
    return nullptr;
  }

  libk::VirtualPA sub_table_va;
  if (!mapper->resolve_pa(get_table_pa_from_entry(entry), &sub_table_va)) {
    // Unable to convert the physical address back to a virtual one :/
    return nullptr;
  }

  return sub_table_va.as_ptr<uint64_t>();
}

static inline constexpr uint64_t get_index_in_table(const libk::VirtualPA& va, size_t level) {
  const size_t index_shift = 12 + 9 * (4 - level);
  // We mask the 9 first bits.
  return (va.as_int() >> index_shift) & libk::mask_bits(0, 8);
}

static inline constexpr libk::VirtualPA get_entry_va_from_table_index(const libk::VirtualPA& table_va,
                                                                      size_t table_level,
                                                                      size_t entry_index) {
  return table_va + (entry_index << (12 + 9 * (4 - table_level)));
}

static inline constexpr libk::VirtualPA table_last_page(const libk::VirtualPA& table_first_page_va,
                                                        size_t table_level) {
  // Check that the virtual address is correctly aligned with the specified level.
  assert((table_first_page_va & libk::mask_bits(0, 12 + 9 * (4 - table_level) - 1)) == 0);

  return table_first_page_va + libk::mask_bits(12, 12 + 9 * (4 - table_level + 1) - 1);
}

uint64_t MMUTable::encode_new_entry(const libk::PhysicalPA& pa, PagesAttributes attr, Amount amount) const {
  const uint64_t upper_attr = (uint64_t)attr.exec << 53;

  const uint64_t lower_attr =
      ((uint64_t)attr.type << 2) | ((uint64_t)attr.access << 6) | ((uint64_t)attr.rw << 7) | ((uint64_t)attr.sh << 8) |
      ((uint64_t)1 << 10)                                   // Set the Access flag to disable access interrupts
      | ((uint64_t)(kind == Kind::Process ? 1 : 0) << 11);  // If this is a process mapping, we set the Non-Global bit

  const uint64_t marker = amount == Amount::Page_4Kio ? PAGE_MARKER : BLOCK_MARKER;

  const uint64_t new_entry = upper_attr | (pa.as_int() & libk::mask_bits(12, 47)) | lower_attr | marker;

  return new_entry;
}

inline void MMUTable::invalid_va(const libk::VirtualPA& entry_va) const {
  const uint64_t page_index = (entry_va.as_int() & libk::mask_bits(12, 47)) >> 12;

  switch (kind) {
    case Kind::Kernel: {
      // Encoding from page D8-6757
      asm volatile("tlbi vaae1, %x0" ::"r"(page_index));
      break;
    }
    case Kind::Process: {
      // Encoding from page D8-6757
      const uint64_t tmp = ((uint64_t)asid << 48) | page_index;
      asm volatile("tlbi vae1, %x0" ::"r"(tmp));
      break;
    }
  }
}

uint64_t* MMUTable::find_table_for_entry(const libk::VirtualPA& entry_va,
                                         size_t entry_level,
                                         bool create_table_if_missing) const {
  if (mapper == nullptr || alloc == nullptr) {
    return nullptr;
  }

  if (kind == Kind::Kernel && (entry_va & TTBR_MASK) != KERNEL_BASE) {
    // Invalid kernel address
    return nullptr;
  };

  if (kind == Kind::Process && (entry_va & TTBR_MASK) != PROCESS_BASE) {
    // Invalid process address
    return nullptr;
  }

  // Check that the virtual address is correctly aligned with the specified amount mapped.
  if ((entry_va & libk::mask_bits(0, 12 + 9 * (4 - entry_level) - 1)) != 0) {
    return nullptr;
  }

  size_t current_level = 1;
  auto* current_table = pgd.as_ptr<uint64_t>();

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
        libk::VirtualPA new_table;
        if (!alloc->alloc_page(&new_table)) {
          // Unable to allocate a fresh table :/
          return nullptr;
        }

        // We resolve its physical address
        libk::PhysicalPA new_table_pa;
        if (!mapper->resolve_va(new_table, &new_table_pa)) {
          return nullptr;
        }

        // And fill the current table with the new entry
        current_table[current_table_index] = new_table_pa.as_int() | TABLE_MARKER;

        // We update the current table now
        current_table = new_table.as_ptr<uint64_t>();
        break;
      }
      case EntryKind::Table: {
        current_table = get_table_ptr_from_entry(entry, mapper);

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

bool MMUTable::map_chunk(const libk::VirtualPA& va,
                         const libk::PhysicalPA& pa,
                         MMUTable::Amount amount,
                         PagesAttributes attr) {
  const auto entry_level = (size_t)amount;
  uint64_t* table_for_entry = find_table_for_entry(va, entry_level, true);

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

  const uint64_t new_entry = encode_new_entry(pa, attr, amount);
  table_for_entry[entry_index] = new_entry;

  return true;
}

bool MMUTable::unmap_chunk(const libk::VirtualPA& va, MMUTable::Amount amount) {
  const auto table_level = (size_t)amount;
  uint64_t* table = find_table_for_entry(va, table_level, false);

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

      invalid_va(va);
      return true;
    }
  }

  return true;  // Why GCC ?!
}

bool MMUTable::has_entry_at(const libk::VirtualPA& va, MMUTable::Amount amount) const {
  const auto entry_level = (size_t)amount;
  return find_table_for_entry(va, entry_level, false) != nullptr;
}

/** All bound are *INCLUSIVE* */
bool MMUTable::map_range(const libk::VirtualPA& va_start,
                         const libk::VirtualPA& va_end,
                         const libk::PhysicalPA& pa_start,
                         PagesAttributes attr) {
  if (va_start > va_end) {
    return false;
  }

  if (va_start == va_end) {
    return map_chunk(va_start, pa_start, Amount::Page_4Kio, attr);
  }

  const auto initial_va = libk::VirtualPA(kind == Kind::Kernel ? KERNEL_BASE : PROCESS_BASE);

  return map_range_in_table(va_start, va_end, pa_start, attr, pgd.as_ptr<uint64_t>(), 1, initial_va);
}

/** All bound are *INCLUSIVE* */
bool MMUTable::map_range_in_table(const libk::VirtualPA& va_start,
                                  const libk::VirtualPA& va_end,
                                  const libk::PhysicalPA& pa_start,
                                  PagesAttributes attr,
                                  uint64_t* table,
                                  size_t table_level,
                                  const libk::VirtualPA& table_first_page_va) {
  if (va_start > va_end || mapper == nullptr || alloc == nullptr) {
    return false;
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
          const uint64_t new_entry = encode_new_entry(entry_pa, attr, Amount::Page_4Kio);
          table[index] = new_entry;
        } else {
          // table_level < 4
          const auto entry_va_end = table_last_page(entry_va_start, table_level + 1);

          if (va_start <= entry_va_start && entry_va_end <= va_end && table_level >= 2) {
            // Map with a block
            const size_t offset = entry_va_start - va_start;
            const auto entry_pa = pa_start + offset;
            const uint64_t new_entry = encode_new_entry(entry_pa, attr, (Amount)table_level);
            table[index] = new_entry;
          } else {
            // Map with a table

            // We need a new page, let's allocate it !
            libk::VirtualPA new_table;
            if (!alloc->alloc_page(&new_table)) {
              // Unable to allocate a fresh table :/
              return false;
            }

            // We resolve its physical address
            libk::PhysicalPA new_table_pa;
            if (!mapper->resolve_va(new_table, &new_table_pa)) {
              return false;
            }

            // And fill the current table with the new entry
            table[index] = new_table_pa.as_int() | TABLE_MARKER;

            // And recursively map what's needed.
            if (!map_range_in_table(va_start, va_end, pa_start, attr, new_table.as_ptr<uint64_t>(), table_level + 1,
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
        uint64_t* sub_table = get_table_ptr_from_entry(entry, mapper);

        if (sub_table == nullptr) {
          // Cannot do any mapping in the table :/
          return false;
        }

        // And recursively map what's needed.
        if (!map_range_in_table(va_start, va_end, pa_start, attr, sub_table, table_level + 1, entry_va_start)) {
          return false;
        }
      }
    }
  }

  return true;
}

void MMUTable::clear_all() {
  const auto initial_va = libk::VirtualPA(kind == Kind::Kernel ? KERNEL_BASE : PROCESS_BASE);

  clear_table(pgd.as_ptr<uint64_t>(), 1, initial_va);
}

void MMUTable::clear_table(uint64_t* table, size_t table_level, const libk::VirtualPA& table_va) {
  for (size_t i = 0; i < TABLE_ENTRIES; ++i) {
    const uint64_t entry = table[i];
    const libk::VirtualPA entry_va = get_entry_va_from_table_index(table_va, table_level, i);
    table[i] = 0ull;  // Clear entry

    switch (get_entry_kind(entry, table_level)) {
      case EntryKind::Invalid: {
        // Nothing else to do good !
        break;
      }
      case EntryKind::Table: {
        uint64_t* sub_table = get_table_ptr_from_entry(entry, mapper);

        if (sub_table != nullptr) {
          clear_table(sub_table, table_level + 1, entry_va);
          alloc->free_page(libk::VirtualPA((uintptr_t)sub_table));
        }
        break;
      }

      case EntryKind::Page:
      case EntryKind::Block: {
        invalid_va(entry_va);
        break;
      }
    }
  }
}
