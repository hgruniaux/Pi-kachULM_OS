#pragma once

#include <cstddef>
#include <cstdint>
#include "libk/memory_page.hpp"
#include "libk/utils.hpp"
#include "mmu_defs.hpp"

// TODO : Add Doc

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

static inline constexpr libk::PhysicalPA get_table_pa_from_entry(uint64_t entry) {
  return libk::PhysicalPA(entry & libk::mask_bits(12, 47));
}


static inline constexpr uint64_t get_index_in_table(libk::VirtualPA va, size_t level) {
  const size_t index_shift = 12 + 9 * (4 - level);
  // We mask the 9 first bits.
  return (va >> index_shift) & libk::mask_bits(0, 8);
}

static inline constexpr libk::VirtualPA get_entry_va_from_table_index(libk::VirtualPA table_va,
                                                                      size_t table_level,
                                                                      size_t entry_index) {
  return table_va + (entry_index << (12 + 9 * (4 - table_level)));
}

static inline constexpr libk::VirtualPA table_last_page(libk::VirtualPA table_first_page_va,
                                                        size_t table_level) {
  // Check that the virtual address is correctly aligned with the specified level.
  KASSERT((table_first_page_va & libk::mask_bits(0, 12 + 9 * (4 - table_level) - 1)) == 0);

  return table_first_page_va + libk::mask_bits(12, 12 + 9 * (4 - table_level + 1) - 1);
}

uint64_t encode_new_entry(libk::PhysicalPA pa, PagesAttributes attr, PageSize amount, bool set_nG);
