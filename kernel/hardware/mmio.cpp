#include "mmio.hpp"
#include <cstdint>

namespace MMIO {
uint64_t BASE;

void init() {
  // TODO: Change BASE to 0xFE000000 when under Raspiberry PI 4
  BASE = 0x3F000000;
}
}  // namespace MMIO
