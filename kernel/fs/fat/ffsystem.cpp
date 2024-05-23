#include "ff.h"

#include "memory/mem_alloc.hpp"

extern "C" {
void* ff_memalloc(UINT byte_count) {
  return kmalloc((size_t)byte_count, alignof(max_align_t));
}

void ff_memfree(void* ptr) {
  kfree(ptr);
}
}
