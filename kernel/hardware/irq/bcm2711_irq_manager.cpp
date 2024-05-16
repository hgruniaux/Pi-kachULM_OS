#include "bcm2711_irq_manager.hpp"
#include "hardware/kernel_dt.hpp"

static uintptr_t _base;

void BCM2711_IRQManager::init() {
  _base = KernelDT::force_get_device_address("intc");
}

void BCM2711_IRQManager::enable_irq(uint64_t irq_id) {}

void BCM2711_IRQManager::disable_irq(uint64_t irq_id) {}

void BCM2711_IRQManager::mask_as_processed(uint64_t irq_id) {}

bool BCM2711_IRQManager::has_pending_interrupt(uint64_t* irq_id) {
  return false;
}
