#include "mmio.hpp"

#include "../debug.hpp"
#include "../mini_clib.hpp"

#include "dtb/dtb.hpp"
#include "dtb/node.hpp"

#include <cstdint>

namespace MMIO {
uint64_t BASE;
DeviceType device;

void init(const DeviceTree& dt) {
  Property p = {};

  if (!dt.find_property("/model", &p)) {
    LOG_CRITICAL("Unable to get model device model.");
  }

  // Yes this is a scrappy way...
  /* TODO :
   * Really use the Device Tree and stop using this MMIO Interface.
   * Configure each device with the Device Tree informations about memory mapping.
   * For instance, to get all information about mailbox :
   *    1. Search for mailbox alias by looking at property /aliases/mailbox
   *    2. Use the information node at path $(mailbox alias)
   *       [spoiler: it's in /soc, whose indications define the memory mapping between the devices and the SOC].
   */
  if (strcmp(p.data, "Raspberry Pi 3 Model B Plus Rev 1.4") == 0) {
    BASE = 0x3F000000;
    device = DeviceType::RaspberryPi3;
  } else if (strcmp(p.data, "Raspberry Pi 4 Model B Rev 1.5") == 0) {
    BASE = 0xFE000000;
    device = DeviceType::RaspberryPi4;
  } else {
    LOG_CRITICAL("Unable to recognize this raspberry pi.");
  }
}
}  // namespace MMIO
