#pragma once

#include "irq_manager.hpp"

namespace BCM2711_IRQManager {
void init();

void enable_irq(uint64_t irq_id);
void disable_irq(uint64_t irq_id);

void mask_as_processed(uint64_t irq_id);
bool has_pending_interrupt(uint64_t* irq_id);
};  // namespace BCM2711_IRQManager
