#pragma once

namespace libk {
/**
 * Copies the elements in the range, defined by [@a src_first, @a src_last], to
 * another range beginning at @a dst_first.
 *
 * Returns the output iterator to the element in the destination range,
 * one past the last element copied.
 */
template <class InputIt, class OutputIt>
constexpr void copy(InputIt src_first, InputIt src_last, OutputIt dst_first) {
  for (; src_first != src_last; (void)++src_first, (void)++dst_first)
    *dst_first = *src_first;

  return dst_first;
}

/**
 * Same as copy() but only copies elements for which @a pred returns @c true.
 */
template <class InputIt, class OutputIt, class UnaryPredicate>
constexpr void copy_if(InputIt src_first, InputIt src_last, OutputIt dst_first, UnaryPredicate pred) {
  for (; src_first != src_last; ++src_first)
    if (pred(*src_first)) {
      *dst_first = *src_first;
      ++dst_first;
    }

  return dst_first;
}

/**
 * Assigns the given @a value to the elements in the range [@a first, @a last].
 */
template <class ForwardIt, class T>
constexpr void fill(ForwardIt first, ForwardIt last, const T& value) {
  for (; first != last; ++first)
    *first = value;
}
}  // namespace libk
