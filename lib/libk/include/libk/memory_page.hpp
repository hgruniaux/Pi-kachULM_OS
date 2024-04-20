#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>

#include "libk/assert.hpp"
#include "libk/utils.hpp"
extern "C" void debug(uint64_t, bool);

namespace libk {

using PhysicalPA = uintptr_t;
using VirtualPA = uintptr_t;

/** This is what we expect of a Page Allocator */
class PageAllocator {
 public:
  /** If allocation successful, modify @a addr to point to a fresh VirtualPA page and returns `true`.
   * Otherwise, @a addr is left untouched, and the function returns `false` */
  [[nodiscard]] virtual bool alloc_page(VirtualPA* addr) = 0;

  /** Free the page at address @a addr. If page is not allocated, then do nothing. */
  virtual void free_page(VirtualPA addr) = 0;
};

/** This is what we expect of a Page Mapping */
class PageMapper {
 public:
  /** Convert a physical address to a virtual one.
   * @returns - `true` if conversion successful, in this case, @a va is modified @n
   *          - `false` if conversion cannot be operated. @a va is unmodified.*/
  [[nodiscard]] virtual bool resolve_pa(PhysicalPA pa, VirtualPA* va) const = 0;

  /** Convert a virtual address to a physical one. @a acc and @a rw give the context for the resolution.
   * @returns - `true` if conversion successful, in this case, @a pa is modified @n
   *          - `false` if conversion cannot be operated. @a pa is unmodified.*/
  [[nodiscard]] virtual bool resolve_va(VirtualPA va, PhysicalPA* pa) const = 0;
};

}  // namespace libk
