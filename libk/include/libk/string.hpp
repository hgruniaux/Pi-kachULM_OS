#pragma once

#include <cstddef>
#include <type_traits>

#include "assert.hpp"

namespace libk {
namespace detail {
// Optimized strlen implementation.
size_t strlen(const char*);
}  // namespace detail

/**
 * Returns the length of the NUL-terminated @a text.
 */
constexpr size_t strlen(const char* text) {
  KASSERT(text != nullptr);

  if (std::is_constant_evaluated()) {
    // Use of a naive implementation understandable by the compiler when constant evaluated,
    // so the compiler can evaluate it at compile time.
    size_t length = 0;
    for (const char* it = text; *it != '\0'; ++it)
      ++length;
    return length;
  } else {
    return detail::strlen(text);
  }
}

char* strcpy(char* dst, const char* src);
void* memset(void* dst, int value, size_t length);
void* memcpy(void* dst, const void* src, size_t length);
}  // namespace libk
