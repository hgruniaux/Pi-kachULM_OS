#include "mmio.hpp"
#include <cstdint>

namespace MMIO {
uint64_t BASE;

void init() {
#if RASPI_VERSION == 3
  BASE = 0x3F000000;
#elif RASPI_VERSION == 4
  BASE = 0xFE000000;
#else
#error the macro RASPI_VERSION must be defined either to 3 or 4
#endif
}
}  // namespace MMIO
