#include <string.h>

void* memmove(void* dst, const void* src, size_t length) {
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
