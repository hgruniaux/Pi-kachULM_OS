#include <string.h>

void* memset(void* dst, int value, size_t length) {
  unsigned char* p = (unsigned char*)dst;
  while (length-- > 0)
    *p++ = (unsigned char)value;
  return dst;
}
