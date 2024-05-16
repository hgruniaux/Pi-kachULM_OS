#pragma once

#include "irq_lists.hpp"
#include "irq_manager.hpp"

namespace BCM2837_IRQManager {
void init();

void enable_irq(IRQ irq_id);
void disable_irq(IRQ irq_id);

void mask_as_processed(IRQ irq_id);
bool has_pending_interrupt(IRQ* irq_id);
};  // namespace BCM2837_IRQManager
