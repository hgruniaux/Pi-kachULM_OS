#pragma once

#include <compare>
#include <cstddef>
#include <memory>
#include "string.hpp"
#include "utils.hpp"

namespace libk {
class StringView {
 public:
  using value_type = char;
  using pointer = char*;
  using const_pointer = const char*;
  using reference = char&;
  using const_reference = const char&;
  using const_iterator = const char*;
  using iterator = const_iterator;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  static inline constexpr size_t npos = -1;

  constexpr StringView() = default;
  constexpr StringView(const char* str) : m_data(str), m_length(strlen(str)) {}
  constexpr StringView(const char* str, size_t length) : m_data(str), m_length(length) {}
  constexpr StringView(const_iterator begin, const_iterator end) : m_data(begin), m_length(end - begin) {}

  /** @brief Checks if the string is empty. */
  [[nodiscard]] constexpr bool is_empty() const { return m_length == 0; }
  /** @brief Returns the string length. */
  [[nodiscard]] constexpr size_t get_length() const { return m_length; }
  /** @brief Returns the string data (may not be NUL terminated). */
  [[nodiscard]] constexpr const char* get_data() const { return m_data; }

  /** @brief Checks if this string starts with the given @a substring. */
  [[nodiscard]] bool starts_with(StringView substring) const {
    if (substring.m_length > m_length)
      return false;
    return strncmp(m_data, substring.m_data, substring.m_length) == 0;
  }

  /** @brief Checks if this string ends with the given @a substring. */
  [[nodiscard]] bool ends_with(StringView substring) const {
    if (substring.m_length > m_length)
      return false;
    return strncmp(m_data + (m_length - substring.m_length), substring.m_data, substring.m_length) == 0;
  }

  /** @brief Finds the first substring equal to the given character sequence. */
  constexpr size_t find(libk::StringView needle, size_t pos = 0) const noexcept {
    if (needle.get_length() == 0) {
      return libk::StringView::npos;
    }

    while (pos < get_length()) {
      if (*(get_data() + pos) == *(needle.get_data()) &&
          libk::strncmp(get_data() + pos, needle.get_data(), needle.get_length()) == 0) {
        return pos;
      }

      pos++;
    }

    return libk::StringView::npos;
  }
  constexpr size_type find(const char* s, size_type pos, size_type count) const {
    return find(StringView(s, count), pos);
  }
  constexpr size_type find(const char* s, size_type pos = 0) const { return find(StringView(s), pos); }

  /** @brief Finds the last occurrence of @a ch. */
  [[nodiscard]] constexpr const_iterator rfind(char ch) const {
    if (is_empty())
      return end();

    for (const_iterator it = end() - 1; it >= begin(); --it) {
      if (*it == ch)
        return it;
    }

    return end();
  }

  /**
   * @brief Compares two character sequences.
   * @see https://en.cppreference.com/w/cpp/string/basic_string_view/compare
   */
  [[nodiscard]] int compare(StringView rhs) const {
    const auto cmp = strncmp(get_data(), rhs.get_data(), min(get_length(), rhs.get_length()));
    if (cmp < 0)
      return cmp;
    if (cmp > 0)
      return cmp;

    if (get_length() < rhs.get_length())
      return -1;
    if (get_length() > rhs.get_length())
      return 1;
    return 0;
  }

  [[nodiscard]] constexpr char operator[](size_t i) const {
    KASSERT(i < m_length);
    return m_data[i];
  }

  // STL interface
  [[nodiscard]] constexpr const_iterator begin() const { return m_data; }
  [[nodiscard]] constexpr const_iterator cbegin() const { return m_data; }
  [[nodiscard]] constexpr const_iterator end() const { return m_data + m_length; }
  [[nodiscard]] constexpr const_iterator cend() const { return m_data + m_length; }

 private:
  const char* m_data = nullptr;
  size_t m_length = 0;
};

[[nodiscard]] inline bool operator==(StringView lhs, StringView rhs) {
  if (lhs.get_length() != rhs.get_length())
    return false;
  return strncmp(lhs.get_data(), rhs.get_data(), lhs.get_length()) == 0;
}

[[nodiscard]] inline std::strong_ordering operator<=>(StringView lhs, StringView rhs) {
  const auto cmp = lhs.compare(rhs);
  if (cmp < 0)
    return std::strong_ordering::less;
  if (cmp > 0)
    return std::strong_ordering::greater;
  return std::strong_ordering::equal;
}
}  // namespace libk
