#include "libk/string.hpp"

extern "C" size_t __strlen_aarch64(const char*);
extern "C" char* __strcpy_aarch64(char*, const char*);
extern "C" int __strcmp_aarch64(const char*, const char*);
extern "C" int __strncmp_aarch64(const char*, const char*, size_t);
extern "C" char* __strrchr_aarch64(const char*, int);
extern "C" char* __strchrnul_aarch64(const char*, int);
extern "C" void* __memchr_aarch64(const void*, int, size_t);
extern "C" int __memcmp_aarch64(const void*, const void*, size_t);
extern "C" void* __memcpy_aarch64(void*, const void*, size_t);
extern "C" void* __memset_aarch64(void*, int, size_t);
extern "C" void* __memmove_aarch64_mops(void*, const void*, size_t);

namespace libk {
size_t strlen(const char* text) {
  return __strlen_aarch64(text);
}

char* strcpy(char* dst, const char* src) {
  return __strcpy_aarch64(dst, src);
}

int strcmp(const char* lhs, const char* rhs) {
  return __strcmp_aarch64(lhs, rhs);
}

int strncmp(const char* lhs, const char* rhs, size_t count) {
  return __strncmp_aarch64(lhs, rhs, count);
}

char* strrchr(const char* str, int ch) {
  return __strrchr_aarch64(str, ch);
}

char* strchrnul(const char* str, int ch) {
  return __strchrnul_aarch64(str, ch);
}

void* memchr(const void* ptr, int value, size_t length) {
  return __memchr_aarch64(ptr, value, length);
}

int memcmp(const void* lhs, const void* rhs, size_t length) {
  return __memcmp_aarch64(lhs, rhs, length);
}

void* memset(void* dst, int value, size_t length) {
  return __memset_aarch64(dst, value, length);
}

void* memcpy(void* dst, const void* src, size_t length) {
  return __memcpy_aarch64(dst, src, length);
}

void* memmove(void* dst, const void* src, size_t length) {
  return __memmove_aarch64_mops(dst, src, length);
}
}  // namespace libk

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
