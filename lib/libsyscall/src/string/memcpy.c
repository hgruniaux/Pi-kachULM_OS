#include <string.h>

void* memcpy(void* dst, const void* src, size_t length) {
  char* d = (char*)dst;
  const char* s = (const char*)src;
  while (length-- > 0)
    *d++ = *s++;
  return dst;
}
