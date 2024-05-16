#pragma once

#include <cstddef>
#include <cstdint>

struct IRQ {
  enum class Type { ARMCore, VideoCore };

  Type type;
  uint64_t id;
};

namespace IRQManager {
using IRQCallBack = void (*)(void*);

void init();

/** Enable IRQ Interrupts */
void enable_irq_interrupts();

/** Disable IRQ Interrupts */
void disable_irq_interrupts();

/** Handle All Interrupts. */
void handle_interrupts();

/** Add the handler @& callback in order to handle the irq @a irq_id.
 * Older handler is replaced. */
void register_irq_handler(IRQ irq, IRQCallBack callback, void* callback_handle);

/** Remove the handler for the irq @a irq_id and deactivate it.
 * If @a callback or @a callback_handle are not null, they are filled with the removed hander. */
void unregister_irq_handle(IRQ irq, IRQCallBack* callback, void** callback_handle);

/** Activate the IRQ */
void activate_irq(IRQ irq);

/** Deactivate the IRQ */
void deactivate_irq(IRQ irq);

};  // namespace IRQManager
