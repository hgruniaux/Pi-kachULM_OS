#include "mmio.hpp"
#include "../memory/mmu_defs.hpp"

namespace MMIO {
uintptr_t BASE;
DeviceType device;


void init() {
#if 1
  BASE = KERNEL_BASE + 0x0000'2000'0000'0000;
  device = DeviceType::RaspberryPi3;
#else
  BASE = 0xFE000000 + KERNEL_BASE;
  device = DeviceType::RaspberryPi4;
#endif
}
}  // namespace MMIO
