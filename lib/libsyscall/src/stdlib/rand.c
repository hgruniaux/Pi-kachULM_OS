#include <stdlib.h>

static uint32_t __rand_seed = 0xDEADBEEF;

void srand(unsigned int seed) {
  __rand_seed = seed;
}

int rand(void) {
  __rand_seed = (48271u * __rand_seed) % ((1 << 31u) - 1u);
  return (int)__rand_seed;
}
