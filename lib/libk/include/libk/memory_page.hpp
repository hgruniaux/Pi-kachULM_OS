#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>

#include "libk/assert.hpp"
#include "libk/utils.hpp"

namespace libk {

#define PageAddressClass(Name)                                     \
  class Name {                                                     \
   public:                                                         \
    inline constexpr explicit Name(uintptr_t ptr) : _ptr(ptr) {    \
      KASSERT((_ptr & mask_bits(0, 12)) == 0);                     \
    }                                                              \
    inline constexpr Name() : _ptr(0) {}                           \
    inline constexpr Name(const Name& other) : _ptr(other._ptr) {} \
                                                                   \
    inline constexpr Name& operator=(const Name& rhs) {            \
      if (this == &rhs) {                                          \
        return *this;                                              \
      }                                                            \
                                                                   \
      _ptr = rhs._ptr;                                             \
      return *this;                                                \
    }                                                              \
                                                                   \
    inline constexpr auto operator==(const Name& rhs) const {      \
      return _ptr == rhs._ptr;                                     \
    }                                                              \
    inline constexpr auto operator<=>(const Name& rhs) const {     \
      return _ptr <=> rhs._ptr;                                    \
    }                                                              \
    inline constexpr auto operator==(const uintptr_t rhs) const {  \
      return _ptr == rhs;                                          \
    }                                                              \
                                                                   \
    inline constexpr Name operator|(uintptr_t address) const {     \
      return Name(_ptr | address);                                 \
    }                                                              \
    inline constexpr Name operator&(uintptr_t address) const {     \
      return Name(_ptr & address);                                 \
    }                                                              \
                                                                   \
    inline constexpr Name operator|(const Name& address) const {   \
      return Name(_ptr | address._ptr);                            \
    }                                                              \
    inline constexpr Name operator&(const Name& address) const {   \
      return Name(_ptr & address._ptr);                            \
    }                                                              \
                                                                   \
    inline constexpr Name operator+(size_t offset) const {         \
      return Name(_ptr + offset);                                  \
    }                                                              \
    inline constexpr Name operator-(size_t offset) const {         \
      return Name(_ptr - offset);                                  \
    }                                                              \
                                                                   \
    inline constexpr size_t operator-(const Name& other) const {   \
      KASSERT(_ptr >= other._ptr);                                 \
      return _ptr - other._ptr;                                    \
    }                                                              \
                                                                   \
    inline constexpr Name& operator+=(size_t offset) {             \
      _ptr += offset;                                              \
      return *this;                                                \
    }                                                              \
    inline constexpr Name& operator-=(size_t offset) {             \
      _ptr -= offset;                                              \
      return *this;                                                \
    }                                                              \
                                                                   \
    [[nodiscard]] inline constexpr uintptr_t as_int() const {      \
      return _ptr;                                                 \
    }                                                              \
                                                                   \
    template <class T>                                             \
    [[nodiscard]] inline constexpr T* as_ptr() const {             \
      return (T*)_ptr;                                             \
    }                                                              \
                                                                   \
   private:                                                        \
    uintptr_t _ptr;                                                \
  }

PageAddressClass(PhysicalPA);
PageAddressClass(VirtualPA);

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
