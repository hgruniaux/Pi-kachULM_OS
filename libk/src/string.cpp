#include "libk/string.hpp"

#if __ARM_FEATURE_SVE
extern "C" size_t __strlen_aarch64_sve(const char*);
extern "C" char* __strcpy_aarch64_sve(char*, const char*);
extern "C" void* __memcpy_aarch64_sve(void*, const void*, size_t);
#else
extern "C" size_t __strlen_aarch64(const char*);
extern "C" char* __strcpy_aarch64(char*, const char*);
extern "C" void* __memcpy_aarch64(void*, const void*, size_t);
#endif
extern "C" void* __memset_aarch64(void*, int, size_t);

namespace libk {
namespace detail {
size_t strlen(const char* text) {
#if __ARM_FEATURE_SVE
  return __strlen_aarch64_sve(text);
#else
  return __strlen_aarch64(text);
#endif
}
}  // namespace detail

char* strcpy(char* dst, const char* src) {
#if __ARM_FEATURE_SVE
  return __strcpy_aarch64_sve(dst, src);
#else
  return __strcpy_aarch64(dst, src);
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
}  // namespace libk

#if LIBK_ENABLE_C
extern "C" size_t strlen(const char* text) {
  return libk::detail::strlen(text);
}

extern "C" char* strcpy(char* dst, const char* src) {
  return libk::strcpy(dst, src);
}

extern "C" void* memset(void* dst, int value, size_t length) {
  return libk::memset(dst, value, length);
}

extern "C" void* memcpy(void* dst, const void* src, size_t length) {
  return libk::memcpy(dst, src, length);
}
#endif  // LIBK_ENABLE_C
