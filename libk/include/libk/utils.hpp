#pragma once

#include <type_traits>

namespace libk {
/**
 * Returns the minimum of @a a and @a b.
 *
 * It uses the @c < operator.
 */
template <class T>
static constexpr T min(T a, T b) {
  return (a < b) ? a : b;
}

/**
 * Returns the maximum of @a a and @a b.
 *
 * It uses the @c < operator.
 */
template <class T>
static constexpr T max(T a, T b) {
  return (a < b) ? b : a;
}
}  // namespace libk
