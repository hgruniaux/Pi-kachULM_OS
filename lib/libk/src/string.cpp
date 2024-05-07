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

/*
 * Testing
 */

#include <libk/test.hpp>

TEST("libk.strlen") {
  EXPECT_EQ(libk::strlen(""), 0);
  EXPECT_EQ(libk::strlen("hello"), 5);
}

TEST("libk.strcmp") {
  EXPECT_EQ(libk::strcmp("hello", "hello"), 0);
  EXPECT_LT(libk::strcmp("hello", "world"), 0);
  EXPECT_LT(libk::strcmp("hello", "hello world"), 0);
  EXPECT_GT(libk::strcmp("hello world", "hello"), 0);
  EXPECT_LT(libk::strcmp("", "hello"), 0);
}

TEST("libk.strchr") {
  const char* str = "hello";
  EXPECT_EQ(libk::strchr(str, 'l'), str + 2);
  EXPECT_EQ(libk::strchr(str, 'z'), nullptr);
  EXPECT_EQ(libk::strchr(str, '\0'), str + 5);
}

TEST("libk.strrchr") {
  const char* str = "hello";
  EXPECT_EQ(libk::strrchr(str, 'l'), str + 3);
  EXPECT_EQ(libk::strrchr(str, 'z'), nullptr);
  EXPECT_EQ(libk::strrchr(str, '\0'), str + 5);
}

TEST("libk.strchrnul") {
  const char* str = "hello";
  EXPECT_EQ(libk::strchrnul(str, 'l'), str + 2);
  EXPECT_EQ(libk::strchrnul(str, 'z'), str + 5);
  EXPECT_EQ(libk::strchrnul(str, '\0'), str + 5);
}

TEST("libk.memset") {
  char buffer[] = {1, 2, 3, 4};
  libk::memset(buffer, 10, sizeof(char) * 3);
  EXPECT_EQ(buffer[0], 10);
  EXPECT_EQ(buffer[1], 10);
  EXPECT_EQ(buffer[2], 10);
  EXPECT_EQ(buffer[3], 4);
}

TEST("libk.bzero") {
  char buffer[] = {1, 2, 3, 4};
  libk::bzero(buffer, sizeof(char) * 3);
  EXPECT_EQ(buffer[0], 0);
  EXPECT_EQ(buffer[1], 0);
  EXPECT_EQ(buffer[2], 0);
  EXPECT_EQ(buffer[3], 4);
}

TEST("libk.memcpy") {
  const char src[] = {1, 2, 3};
  char dst[] = {10, 11, 12, 13};
  libk::memcpy(dst, src, sizeof src);
  EXPECT_EQ(dst[0], 1);
  EXPECT_EQ(dst[1], 2);
  EXPECT_EQ(dst[2], 3);
  EXPECT_EQ(dst[3], 13);
}
