#pragma once

#include <cstddef>
#include <cstdint>

namespace IRQManager {
using IRQCallBack = bool (*)(void*);

void init();

/** Enable IRQ Interrupts */
void enable_irq_interrupts();

/** Disable IRQ Interrupts */
void disable_irq_interrupts();

/** Handle All Interrupts.
 * @returns `true` if all interrupts are processed, and false otherwise. */
[[nodiscard]] bool handle_interrupts();

/** Add the handler @& callback in order to handle the irq @a irq_id.
 * Recently added handler are called first, until one that returns `true`. */
[[nodiscard]] bool register_irq_handler(uint64_t irq_id, IRQCallBack callback, void* callback_handle);

/** Remove and fill in @a callback (if not null) the newest handler for the irq @a irq_id.
 * If, no handler are found for this irq, `false` is returned. */
bool unregister_first_irq_handle(uint64_t irq_id, IRQCallBack* callback, void** callback_handle);

/** Remove all handler for the irq @a irq_id and deactivate it. */
void unregister_all_irq_handle(uint64_t irq_id);

/** Activate the IRQ */
void activate_irq(uint64_t irq);

/** Deactivate the IRQ */
void deactivate_irq(uint64_t irq);

};  // namespace IRQManager
