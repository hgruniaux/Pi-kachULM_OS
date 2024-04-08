#include "mmio.hpp"

namespace MMIO {
uint64_t BASE;
DeviceType device;

void init() {
#if 1
  BASE = 0x3F000000;
    device = DeviceType::RaspberryPi3;
#else
  BASE = 0xFE000000;
    device = DeviceType::RaspberryPi4;
#endif
}
}  // namespace MMIO
