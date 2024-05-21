#pragma once

#include <libk/bit_array.hpp>
#include "mmu_table.hpp"

class ContiguousPageAllocator {
 public:
  explicit ContiguousPageAllocator() = default;
  explicit ContiguousPageAllocator(size_t nb_pages, uintptr_t array);

  // Utility functions
  /** @brief Mark @a nb_pages pages starting from @a addr as used */
  void mark_as_used(PhysicalPA start, PhysicalPA end);

  /** Tries to find @a nb_pages contiguous fresh page.
   * @returns   - `true` in case of success, @a start and @a end are filled in this case. @n
   *            - `false` otherwise, @a start and @a end are not modified. */
  bool fresh_pages(size_t nb_pages, PhysicalPA* start, PhysicalPA *end);

  /** @brief Free @a nb_pages pages starting from @a addr. */
  void free_pages(PhysicalPA start, PhysicalPA end);

  /** Check if physical page is free or not
   * @returns   - `true` if page is free @n
   *            - `false` otherwise */
  bool page_status(PhysicalPA addr) const;

  /** @brief Returns the memory needed by this construction to manage
   * @a nb_pages pages in *bytes* */
  static uint64_t memory_needed(size_t nb_pages);

 protected:
  bool try_fresh_pages(size_t nb_pages, PhysicalPA* start, PhysicalPA *end, bool first_pass);
  size_t page_index(PhysicalPA addr) const;

  uint64_t _nb_pages;
  libk::BitArray _free_page;
  PhysicalPA _page_cursor;
};
