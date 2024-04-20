#pragma once

#include <cstddef>
#include <cstdint>

#include <libk/memory_page.hpp>
#include "mmu_defs.hpp"

class MMUTable {
 public:
  enum class Kind : uint8_t { Kernel, Process };

  /** The kind of Memory mapping we're dealing with. */
  const Kind kind;

  /** The allocator used when a new page is necessary. */
  libk::PageAllocator* const alloc;

  /** The mapper used when conversion is needed. */
  libk::PageMapper* const mapper;

  /** The top level of the MMU table. */
  const libk::VirtualPA pgd;

  /** The Address Space Identifier, used to differentiate between process memory spaces. */
  const uint8_t asid;

  /** Maps a chunk of @a amount memory page from @a va to @a pa with attributes @a attr
   * @returns - `true` iff the specified chunk is now mapped. @n
   *          - `false` iff @a va and @a amount does not refer to a atomic chunk of memory
   *          or an error occurred during a page allocation. */
  [[nodiscard]] bool map_chunk(libk::VirtualPA va, libk::PhysicalPA pa, PageSize amount, PagesAttributes attr);

  /** Maps the virtual address range from @a va_start to @a va_end *INCLUDED*
   * to the physical addresses @a pa_start and following, using @a attr attributes.
   * Mapping is performed to minimize the number of tables used.
   *
   * @returns - `true` if complete mapping has been performed successfully. @n
   *          - `false` if some or all of the mapping was not successful.
   *          The table is left in an unknown, possibly invalid, state.
   *
   * WARNING:
   * AFTER THIS OPERATION, YOU WILL HAVE NO KNOWLEDGE OF THE TABLE STRUCTURE FOR THIS VIRTUAL ADDRESS RANGE.
   * Please use this function with caution, only to map an address range that will not be unmapped.
   */
  [[nodiscard]] bool map_range(libk::VirtualPA va_start,
                               libk::VirtualPA va_end,
                               libk::PhysicalPA pa_start,
                               PagesAttributes attr);

  /** Unmap the chunk of @a amount memory starting from @a va.
   * @returns - `true` iff the specified chunk is now unmapped. @n
   *          - `false` iff @a va and @a amount does not refer to a atomic chunk of memory. */
  bool unmap_chunk(libk::VirtualPA va, PageSize amount);

  /** Checks if a there is an entry mapping @a amount of memory starting from @a va. */
  [[nodiscard]] bool has_entry_at(libk::VirtualPA va, PageSize amount) const;

  [[nodiscard]] bool get_entry_attributes(libk::VirtualPA va, PageSize amount, PagesAttributes* attr) const;
  [[nodiscard]] bool set_entry_attributes(libk::VirtualPA va, PageSize amount, PagesAttributes attr);

  /** Clear the whole table, deallocating all used pages and unmapping everything. */
  void clear_all();

 private:
  [[nodiscard]] uint64_t* find_table_for_entry(libk::VirtualPA entry_va,
                                               size_t entry_level,
                                               bool create_table_if_missing) const;

  [[nodiscard]] bool map_range_in_table(libk::VirtualPA va_start,
                                        libk::VirtualPA va_end,
                                        libk::PhysicalPA pa_start,
                                        PagesAttributes attr,
                                        uint64_t* table,
                                        size_t table_level,
                                        libk::VirtualPA table_first_page_va);

  void invalid_va(libk::VirtualPA entry_va) const;

  void clear_table(uint64_t* table, size_t table_level, libk::VirtualPA table_first_page_va);
};

/** Address Translate instruction wrapper.
 * With the help of @a acc and @a rw, call the correct `at` instruction with @a va. */
static inline void at_instr(libk::VirtualPA va, Accessibility acc, ReadWritePermission rw) {
  switch (acc) {
    case Accessibility::Privileged: {
      switch (rw) {
        case ReadWritePermission::ReadWrite:
          asm volatile("at s1e1w, %x0" ::"r"(va));
          return;

        case ReadWritePermission::ReadOnly:
          asm volatile("at s1e1r, %x0" ::"r"(va));
          return;
      }
    }
    case Accessibility::AllProcess: {
      switch (rw) {
        case ReadWritePermission::ReadWrite:
          asm volatile("at s1e0w, %x0" ::"r"(va));
          return;

        case ReadWritePermission::ReadOnly:
          asm volatile("at s1e0r, %x0" ::"r"(va));
          return;
      }
    }
  }
}

/** Retrieve a physical address from PAR_EL1.
 * @returns - `true` if conversion successful, in this case, @a pa is modified @n
 *          - `false` if conversion cannot be operated. @a pa is unmodified. */
static inline bool parse_PAR_EL1(libk::PhysicalPA* pa) {
  uint64_t physical_addr;
  asm volatile("mrs %x0, PAR_EL1" : "=r"(physical_addr));
  if ((physical_addr & 0b1) == 0) {
    *pa = libk::PhysicalPA(physical_addr & libk::mask_bits(47, 12));
    return true;
  } else {
    return false;
  }
}
