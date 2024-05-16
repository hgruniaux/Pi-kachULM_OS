#pragma once

#include <cstddef>
#include <type_traits>

#include "assert.hpp"

namespace libk {
/** Implementation of the C standard `strlen()` function. */
constexpr inline size_t strlen(const char* text) {
  size_t length = 0;
  while (*text++ != '\0')
    ++length;
  return length;
}

/** Implementation of the C standard `strcpy()` function. */
constexpr inline char* strcpy(char* dst, const char* src) {
  char* tmp = dst;
  while ((*dst++ = *src++) != '\0')
    ;
  return tmp;
}

/** Implementation of the C standard `strcmp()` function. */
constexpr inline int strcmp(const char* lhs, const char* rhs) {
  while ((*lhs != '\0') && (*lhs == *rhs)) {
    ++lhs;
    ++rhs;
  }

  return ((int)*lhs - (int)*rhs);
}

/** Implementation of the C standard `strncmp()` function. */
constexpr inline int strncmp(const char* lhs, const char* rhs, size_t count) {
  while (count > 0 && *lhs != '\0' && (*lhs == *rhs)) {
    ++lhs;
    ++rhs;
    --count;
  }

  if (count == 0) {
    return 0;
  } else {
    return ((int)*lhs - (int)*rhs);
  }
}

/** Implementation of the C standard `strchr()` function. */
constexpr inline char* strchr(const char* str, int ch) {
  do {
    if (*str == (char)ch) {
      return (char*)str;
    }
  } while (*str++ != '\0');

  return nullptr;
}

/** Implementation of the C standard `strrchr()` function. */
constexpr inline char* strrchr(const char* str, int ch) {
  size_t i = 0;

  while (str[i++] != '\0')
    ;

  do {
    if (str[--i] == (char)ch) {
      return (char*)str + i;
    }
  } while (i > 0);

  return nullptr;
}

/** Implementation of the POSIX standard `strchrnul()` function. */
constexpr inline char* strchrnul(const char* str, int ch) {
  const char c = (char)(ch);
  for (; (*str != '\0') && *str != c; ++str)
    ;
  return const_cast<char*>(str);
}

/** Implementation of the C standard `strstr()` function. */
constexpr inline const char* strstr(const char* haystack, const char* needle) {
  const size_t needle_length = strlen(needle);

  if (needle_length == 0) {
    return haystack;
  }

  while (*haystack != '\0') {
    if ((*haystack == *needle) && (strncmp(haystack, needle, needle_length) == 0)) {
      return haystack;
    }

    haystack++;
  }

  return nullptr;
}

/** Implementation of the C standard `memchr()` function. */
inline void* memchr(const void* ptr, int value, size_t length) {
  const unsigned char* p = (const unsigned char*)ptr;

  while (length-- > 0) {
    if (*p == (unsigned char)value) {
      return (void*)p;
    }

    ++p;
  }

  return nullptr;
}

/** Implementation of the C standard `memcmp()` function. */
inline int memcmp(const void* lhs, const void* rhs, size_t length) {
  const unsigned char* p1 = (const unsigned char*)lhs;
  const unsigned char* p2 = (const unsigned char*)rhs;

  while (length-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }

    ++p1;
    ++p2;
  }

  return 0;
}

/** Implementation of the C standard `memset()` function. */
inline void* memset(void* dst, int value, size_t length) {
  unsigned char* p = (unsigned char*)dst;
  while (length-- > 0)
    *p++ = (unsigned char)value;
  return dst;
}

/** Zero memory. */
inline void bzero(void* dst, size_t length) {
  memset(dst, 0, length);
}

/** Implementation of the C standard `memcpy()` function. */
inline void* memcpy(void* dst, const void* src, size_t length) {
  char* d = (char*)dst;
  const char* s = (const char*)src;
  while (length-- > 0)
    *d++ = *s++;
  return dst;
}

/** Implementation of the C standard `memmove()` function. */
inline void* memmove(void* dst, const void* src, size_t length) {
  char* d = (char*)dst;
  const char* s = (const char*)src;

  if (d <= s) {
    while (length-- > 0)
      *d++ = *s++;
  } else {
    s += length;
    d += length;

    while (length-- > 0)
      *--d = *--s;
  }

  return dst;
}
}  // namespace libk
