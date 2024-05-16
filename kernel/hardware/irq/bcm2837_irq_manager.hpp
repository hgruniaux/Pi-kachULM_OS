#pragma once

#include "irq_manager.hpp"

namespace BCM2837_IRQManager {
void init();

void enable_irq(uint32_t irq_id);
void disable_irq(uint32_t irq_id);

void mask_as_processed(uint32_t irq_id);
bool has_pending_interrupt(uint32_t* irq_id);
};  // namespace BCM2837_IRQManager
