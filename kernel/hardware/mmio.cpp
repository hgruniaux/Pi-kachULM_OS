#include "mmio.hpp"
#include "../mmu.h"

namespace MMIO {
uintptr_t BASE;
DeviceType device;

void init() {
#if 1
  BASE = 0x3F000000 + KERNEL_BASE;
  device = DeviceType::RaspberryPi3;
#else
  BASE = 0xFE000000 + KERNEL_BASE;
  device = DeviceType::RaspberryPi4;
#endif
}
}  // namespace MMIO
