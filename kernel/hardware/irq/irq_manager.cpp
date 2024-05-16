#include "irq_manager.hpp"
#include "libk/log.hpp"

#include "hardware/kernel_dt.hpp"

#include "bcm2711_irq_manager.hpp"
#include "bcm2837_irq_manager.hpp"

namespace IRQManager {

static void (*_enable_irq)(IRQ);
static void (*_disable_irq)(IRQ);
static void (*_mask_as_processed)(IRQ);
static bool (*_has_pending_interrupt)(IRQ*);

struct CallBackAssoc {
  IRQCallBack cb = nullptr;
  void* cb_handle = nullptr;
};

static CallBackAssoc armc_handler[ARMC_IRQ_NB] = {};
static CallBackAssoc vc_handler[VC_IRQ_NB] = {};

void init() {
  for (const auto comp : KernelDT::get_board_compatible()) {
    if (comp.find("bcm2837") != libk::StringView::npos) {
      _enable_irq = &BCM2837_IRQManager::enable_irq;
      _disable_irq = &BCM2837_IRQManager::disable_irq;
      _mask_as_processed = &BCM2837_IRQManager::mask_as_processed;
      _has_pending_interrupt = &BCM2837_IRQManager::has_pending_interrupt;

      BCM2837_IRQManager::init();
      return;
    }

    if (comp.find("bcm2711") != libk::StringView::npos) {
      _enable_irq = &BCM2711_IRQManager::enable_irq;
      _disable_irq = &BCM2711_IRQManager::disable_irq;
      _mask_as_processed = &BCM2711_IRQManager::mask_as_processed;
      _has_pending_interrupt = &BCM2711_IRQManager::has_pending_interrupt;

      BCM2711_IRQManager::init();
      return;
    }
  }
}

void enable_irq_interrupts() {
  asm volatile("msr daifclr, #2");
}

void disable_irq_interrupts() {
  asm volatile("msr daifset, #2");
}

void handle_interrupts() {
  disable_irq_interrupts();

  IRQ irq;

  while ((*_has_pending_interrupt)(&irq)) {
    LOG_INFO("Got IRQ id: {:#x}, IRQ type: {}", irq.id, (int)irq.type);

    CallBackAssoc cb_assoc;
    switch (irq.type) {
      case IRQ::Type::ARMCore:
        cb_assoc = armc_handler[irq.id];
        break;
      case IRQ::Type::VideoCore:
        cb_assoc = vc_handler[irq.id];
        break;
    }

    LOG_INFO("Found Handler: {:#x} (handle: {:#x})", (void*)(cb_assoc.cb), cb_assoc.cb_handle);

    if (cb_assoc.cb == nullptr) {
      LOG_ERROR("No handler for interrupt: (id: {:#x}, type: {})", irq.id, (int)irq.type);
      libk::panic("IRQ Handler Missing");
    }

    (*cb_assoc.cb)(cb_assoc.cb_handle);
    (*_mask_as_processed)(irq);
  }

  enable_irq_interrupts();
}

void register_irq_handler(IRQ irq, IRQCallBack callback, void* cb_handle) {
  switch (irq.type) {
    case IRQ::Type::ARMCore:
      armc_handler[irq.id] = {callback, cb_handle};
      break;
    case IRQ::Type::VideoCore:
      vc_handler[irq.id] = {callback, cb_handle};
      break;
  }

  activate_irq(irq);
}

void unregister_irq_handle(IRQ irq, IRQCallBack* callback, void** callback_handle) {
  deactivate_irq(irq);

  switch (irq.type) {
    case IRQ::Type::ARMCore: {
      const auto cb_entry = armc_handler[irq.id];

      if (callback != nullptr) {
        *callback = cb_entry.cb;
      }
      if (callback_handle != nullptr) {
        *callback_handle = cb_entry.cb_handle;
      }
      break;
    }

    case IRQ::Type::VideoCore: {
      const auto cb_entry = vc_handler[irq.id];

      if (callback != nullptr) {
        *callback = cb_entry.cb;
      }
      if (callback_handle != nullptr) {
        *callback_handle = cb_entry.cb_handle;
      }
      break;
    }
  }
}

void activate_irq(IRQ irq) {
  (*_enable_irq)(irq);
}

void deactivate_irq(IRQ irq) {
  (*_disable_irq)(irq);
}
}  // namespace IRQManager
