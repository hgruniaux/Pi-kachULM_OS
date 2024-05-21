#include <stdlib.h>
#include <sys/syscall.h>

void* malloc(size_t n) {
  // TODO: better malloc
  return sys_sbrk(n);
}

void free(void* ptr) {
  (void)ptr;
  // TODO: better free
}
