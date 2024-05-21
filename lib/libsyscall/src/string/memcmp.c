#include <string.h>

int memcmp(const void* lhs, const void* rhs, size_t length) {
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
}
