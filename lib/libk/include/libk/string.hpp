#pragma once

#include <cstddef>
#include <type_traits>

#include "assert.hpp"

// Optimized implementations.
extern "C" size_t __strlen_aarch64(const char*);
extern "C" char* __strcpy_aarch64(char*, const char*);
extern "C" int __strcmp_aarch64(const char*, const char*);
extern "C" int __strncmp_aarch64(const char*, const char*, size_t);
extern "C" char* __strchr_aarch64(const char*, int);
extern "C" char* __strrchr_aarch64(const char*, int);
extern "C" char* __strchrnul_aarch64(const char*, int);
extern "C" void* __memchr_aarch64(const void*, int, size_t);
extern "C" int __memcmp_aarch64(const void*, const void*, size_t);
extern "C" void* __memcpy_aarch64(void*, const void*, size_t);
extern "C" void* __memset_aarch64(void*, int, size_t);
extern "C" void* __memmove_aarch64_mops(void*, const void*, size_t);

namespace libk {
/** Implementation of the C standard `strlen()` function. */
constexpr inline size_t strlen(const char* text) {
#if !LIBK_NAIVE_IMPLEMENTATION
  if (std::is_constant_evaluated()) {
#endif  // LIBK_NAIVE_IMPLEMENTATION
    size_t length = 0;
    while (*text++ != '\0')
      ++length;
    return length;
#if !LIBK_NAIVE_IMPLEMENTATION
  } else {
    return __strlen_aarch64(text);
  }
#endif  // LIBK_NAIVE_IMPLEMENTATION
}

/** Implementation of the C standard `strcpy()` function. */
constexpr inline char* strcpy(char* dst, const char* src) {
#if !LIBK_NAIVE_IMPLEMENTATION
  if (std::is_constant_evaluated()) {
#endif  // LIBK_NAIVE_IMPLEMENTATION
    char* tmp = dst;
    while ((*dst++ = *src++) != '\0')
      ;
    return tmp;
#if !LIBK_NAIVE_IMPLEMENTATION
  } else {
    return __strcpy_aarch64(dst, src);
  }
#endif  // LIBK_NAIVE_IMPLEMENTATION
}

/** Implementation of the C standard `strcmp()` function. */
constexpr inline int strcmp(const char* lhs, const char* rhs) {
#if !LIBK_NAIVE_IMPLEMENTATION
  if (std::is_constant_evaluated()) {
#endif  // LIBK_NAIVE_IMPLEMENTATION
    while ((*lhs != '\0') && (*lhs == *rhs)) {
      ++lhs;
      ++rhs;
    }

    return ((int)*lhs - (int)*rhs);
#if !LIBK_NAIVE_IMPLEMENTATION
  } else {
    return __strcmp_aarch64(lhs, rhs);
  }
#endif  // LIBK_NAIVE_IMPLEMENTATION
}

/** Implementation of the C standard `strncmp()` function. */
constexpr inline int strncmp(const char* lhs, const char* rhs, size_t count) {
#if !LIBK_NAIVE_IMPLEMENTATION
  if (std::is_constant_evaluated()) {
#endif  // LIBK_NAIVE_IMPLEMENTATION
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

#if !LIBK_NAIVE_IMPLEMENTATION
  } else {
    return __strncmp_aarch64(lhs, rhs, count);
  }
#endif  // LIBK_NAIVE_IMPLEMENTATION
}

/** Implementation of the C standard `strchr()` function. */
constexpr inline char* strchr(const char* str, int ch) {
#if !LIBK_NAIVE_IMPLEMENTATION
  if (std::is_constant_evaluated()) {
#endif  // LIBK_NAIVE_IMPLEMENTATION
    do {
      if (*str == (char)ch) {
        return (char*)str;
      }
    } while (*str++ != '\0');

    return nullptr;
#if !LIBK_NAIVE_IMPLEMENTATION
  } else {
    return __strchr_aarch64(str, ch);
  }
#endif  // LIBK_NAIVE_IMPLEMENTATION
}

/** Implementation of the C standard `strrchr()` function. */
inline char* strrchr(const char* str, int ch) {
  return __strrchr_aarch64(str, ch);
}

/** Implementation of the C standard `strchrnul()` function. */
inline char* strchrnul(const char* str, int ch) {
  return __strchrnul_aarch64(str, ch);
}

/** Implementation of the C standard `strchrnul()` function. */
inline void* memchr(const void* ptr, int value, size_t length) {
#if LIBK_NAIVE_IMPLEMENTATION
  const unsigned char* p = (const unsigned char*)ptr;

  while (length-- > 0) {
    if (*p == (unsigned char)value) {
      return (void*)p;
    }

    ++p;
  }

  return nullptr;
#else
  return __memchr_aarch64(ptr, value, length);
#endif  // LIBK_NAIVE_IMPLEMENTATION
}

/** Implementation of the C standard `memcmp()` function. */
inline int memcmp(const void* lhs, const void* rhs, size_t length) {
#if LIBK_NAIVE_IMPLEMENTATION
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
#else
  return __memcmp_aarch64(lhs, rhs, length);
#endif  // LIBK_NAIVE_IMPLEMENTATION
}

/** Implementation of the C standard `memset()` function. */
inline void* memset(void* dst, int value, size_t length) {
#if LIBK_NAIVE_IMPLEMENTATION
  unsigned char* p = (unsigned char*)dst;
  while (length-- > 0)
    *p++ = (unsigned char)value;
  return dst;
#else
  return __memset_aarch64(dst, value, length);
#endif  // LIBK_NAIVE_IMPLEMENTATION
}

/** Zero memory. */
inline void bzero(void* dst, size_t length) {
  memset(dst, 0, length);
}

/** Implementation of the C standard `memcpy()` function. */
inline void* memcpy(void* dst, const void* src, size_t length) {
#if LIBK_NAIVE_IMPLEMENTATION
  char* d = (char*)dst;
  const char* s = (const char*)src;
  while (length-- > 0)
    *d++ = *s++;
  return dst;
#else
  return __memcpy_aarch64(dst, src, length);
#endif  // LIBK_NAIVE_IMPLEMENTATION
}

/** Implementation of the C standard `memmove()` function. */
inline void* memmove(void* dst, const void* src, size_t length) {
#if LIBK_NAIVE_IMPLEMENTATION
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
#else
  return __memmove_aarch64_mops(dst, src, length);
#endif  // LIBK_NAIVE_IMPLEMENTATION
}
}  // namespace libk
