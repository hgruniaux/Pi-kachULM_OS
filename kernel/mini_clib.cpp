#include "mini_clib.hpp"
#include <cstddef>
#include "hardware/uart.hpp"

size_t strlen(const char* str) {
  size_t size = 0;

  while (*str++ != 0) {
    size++;
  }

  return size;
}

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
