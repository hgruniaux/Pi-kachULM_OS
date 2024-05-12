#pragma once
#include "libk/bit_array.hpp"
#include "mmu_table.hpp"

class PageAlloc {
 public:
  explicit PageAlloc() = default;
  explicit PageAlloc(size_t nb_pages, uintptr_t array);

  // Utility functions
  /** @brief Mark page as used */
  void mark_as_used(PhysicalPA addr);

  /** Tries to find a fresh page.
   * @returns   - `true` in case of success, @a addr is filled in this case. @n
   *            - `false` otherwise, @a addr is not modified. */
  bool fresh_page(PhysicalPA* addr);

  /** @brief Free the physical page @a addr. */
  void free_page(PhysicalPA addr);

  /** Check if physical page is free or not
   * @returns   - `true` if page is free @n
   *            - `false` otherwise */
  bool page_status(PhysicalPA addr) const;

  /** @brief Returns the memory needed by this construction to manage
   * @a nb_pages pages in *bytes* */
  static uint64_t memory_needed(size_t nb_pages);

 protected:
  inline size_t page_index(PhysicalPA addr) const;
  uint64_t m_nb_pages;
  libk::BitArray m_mmap;
};
