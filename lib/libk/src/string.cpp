#include "libk/string.hpp"

/*
 * Start of the C API:
 */

extern "C" size_t strlen(const char* text) {
  return libk::strlen(text);
}

extern "C" char* strcpy(char* dst, const char* src) {
  return libk::strcpy(dst, src);
}

extern "C" int strcmp(const char* lhs, const char* rhs) {
  return libk::strcmp(lhs, rhs);
}

extern "C" int strncmp(const char* lhs, const char* rhs, size_t count) {
  return libk::strncmp(lhs, rhs, count);
}

extern "C" char* strchr(const char* str, int ch) {
  return libk::strchr(str, ch);
}

extern "C" char* strrchr(const char* str, int ch) {
  return libk::strrchr(str, ch);
}

extern "C" char* strchrnul(const char* str, int ch) {
  return libk::strchrnul(str, ch);
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
