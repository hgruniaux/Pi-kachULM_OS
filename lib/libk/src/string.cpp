#include "libk/string.hpp"

#if __ARM_FEATURE_SVE
extern "C" size_t __strlen_aarch64_sve(const char*);
extern "C" char* __strcpy_aarch64_sve(char*, const char*);
extern "C" void* __memchr_aarch64_sve(const void*, int, size_t);
extern "C" int __memcmp_aarch64_sve(const void*, const void*, size_t);
extern "C" void* __memcpy_aarch64_sve(void*, const void*, size_t);
#else
extern "C" size_t __strlen_aarch64(const char*);
extern "C" char* __strcpy_aarch64(char*, const char*);
extern "C" void* __memchr_aarch64(const void*, int, size_t);
extern "C" int __memcmp_aarch64(const void*, const void*, size_t);
extern "C" void* __memcpy_aarch64(void*, const void*, size_t);
#endif
extern "C" void* __memset_aarch64(void*, int, size_t);
extern "C" void* __memmove_aarch64_mops(void*, const void*, size_t);

namespace libk {

// TODO : Update This
int strcmp(const char* lhs, const char* rhs) {
  // From glibc 2.7
  unsigned char c1, c2;

  do {
    c1 = *lhs++;
    c2 = *rhs++;

    if (c1 == '\0') {
      return c1 - c2;
    }

  } while (c1 == c2);

  return c1 - c2;
}

int strncmp(const char* lhs, const char* rhs, size_t count) {
  // From glibc 2.7
  unsigned char c1 = 0, c2 = 0;

  while (count > 0) {
    c1 = *lhs++;
    c2 = *rhs++;

    if (c1 == '\0' || c1 != c2) {
      return c1 - c2;
    }

    count--;
  }

  return c1 - c2;
}

char* strrchr(const char* str, int ch) {
  if (ch == '\0') {
    while (*str != '\0') {
      str++;
    }

    return (char*)str;
  }

  const char* last_found = nullptr;

  while (*str != '\0') {
    while (!(*str == '\0' || *str == ch)) {
      str++;
    }

    if (*str == ch) {
      last_found = str;
      str++;
    }
  }

  return (char*)last_found;
}

char* strchrnul(const char* str, int ch) {
  while (!(*str == '\0' && *str == ch)) {
    str++;
  }
  return (char*)str;
}
// End of TODO

size_t strlen(const char* text) {
#if __ARM_FEATURE_SVE
  return __strlen_aarch64_sve(text);
#else
  return __strlen_aarch64(text);
#endif
}

char* strcpy(char* dst, const char* src) {
#if __ARM_FEATURE_SVE
  return __strcpy_aarch64_sve(dst, src);
#else
  return __strcpy_aarch64(dst, src);
#endif
}

void* memchr(const void* ptr, int value, size_t length) {
#if __ARM_FEATURE_SVE
  return __memchr_aarch64_sve(ptr, value, length);
#else
  return __memchr_aarch64(ptr, value, length);
#endif
}

int memcmp(const void* lhs, const void* rhs, size_t length) {
#if __ARM_FEATURE_SVE
  return __memcmp_aarch64_sve(lhs, rhs, length);
#else
  return __memcmp_aarch64(lhs, rhs, length);
#endif
}

void* memset(void* dst, int value, size_t length) {
  return __memset_aarch64(dst, value, length);
}

void* memcpy(void* dst, const void* src, size_t length) {
#if __ARM_FEATURE_SVE
  return __memcpy_aarch64_sve(dst, src, length);
#else
  return __memcpy_aarch64(dst, src, length);
#endif
}

void* memmove(void* dst, const void* src, size_t length) {
  return __memmove_aarch64_mops(dst, src, length);
}
}  // namespace libk

extern "C" size_t strlen(const char* text) {
  return libk::strlen(text);
}

extern "C" char* strcpy(char* dst, const char* src) {
  return libk::strcpy(dst, src);
}

extern "C" void* memchr(const void* ptr, int value, size_t length) {
  return libk::memchr(ptr, value, length);
}

extern "C" int memcmp(const void* lhs, const void* rhs, size_t length) {
  return libk::memcmp(lhs, rhs, length);
}

extern "C" void* memset(void* dst, int value, size_t length) {
  return libk::memset(dst, value, length);
}

extern "C" void* memcpy(void* dst, const void* src, size_t length) {
  return libk::memcpy(dst, src, length);
}

extern "C" void* memmove(void* dst, const void* src, size_t length) {
  return libk::memmove(dst, src, length);
}
