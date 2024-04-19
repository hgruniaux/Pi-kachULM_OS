#pragma once

#include <cstddef>
#include <cstdint>
#include <libk/memory_page.hpp>

#include "defs.hpp"

/**
 * This Allocator just allocate memory, page by page.
 * It does not support freeing them !
 */
class SimplePageAllocator : public libk::PageAllocator {
 public:
  /** Construct a @a SimplePageAllocator.
   * It is the responsibility of the user to give a page-aligned pointer ! */
  explicit SimplePageAllocator(uintptr_t begin) : _start(begin), _nb_page(0) {}

  /** Allocate a new page, in order. */
  [[nodiscard]] bool alloc_page(libk::VirtualPA* addr) final {
    *addr = libk::VirtualPA(_start + PAGE_SIZE * _nb_page++);
    return true;
  }

  /** Page freeing is *NOT* supported, so the address in ignored. */
  void free_page(libk::VirtualPA addr) final { (void)addr; }

  /** Returns the address of the first allocated page */
  [[nodiscard]] uintptr_t get_section_start() const { return _start; }

  /** Returns the address of the next allocated page */
  [[nodiscard]] uintptr_t get_section_end() const { return _start + PAGE_SIZE * _nb_page; }

 private:
  const uintptr_t _start;
  size_t _nb_page;
};
