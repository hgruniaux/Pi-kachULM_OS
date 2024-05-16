#include "irq_manager.hpp"
#include <algorithm>
#include "libk/linked_list.hpp"
#include "libk/log.hpp"

#include "hardware/kernel_dt.hpp"

#include "bcm2711_irq_manager.hpp"
#include "bcm2837_irq_manager.hpp"

static void (*_enable_irq)(uint64_t);
static void (*_disable_irq)(uint64_t);
static void (*_mask_as_processed)(uint64_t);
static bool (*_has_pending_interrupt)(uint64_t*);

struct CallBackAssoc {
  uint64_t irq = 0;
  IRQManager::IRQCallBack cb = nullptr;
  void* cb_handle = nullptr;
};

libk::LinkedList<CallBackAssoc> _callbacks;

void IRQManager::init() {
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

void IRQManager::enable_irq_interrupts() {
  asm volatile("msr daifclr, #2");
}

void IRQManager::disable_irq_interrupts() {
  asm volatile("msr daifset, #2");
}

bool IRQManager::handle_interrupts() {
  uint64_t irq_id = -1;

  while ((*_has_pending_interrupt)(&irq_id)) {
    auto it = _callbacks.begin();
    bool handled = false;

    while (it != std::end(_callbacks) && !handled) {
      if (it->irq == irq_id) {
        handled = (*it->cb)(it->cb_handle);
      }

      ++it;
    }

    if (!handled) {
      return false;
    }

    (*_mask_as_processed)(irq_id);
  }

  return true;
}

bool IRQManager::register_irq_handler(uint64_t irq_id, IRQCallBack callback, void* cb_handle) {
  if (callback == nullptr) {
    return false;
  }

  _callbacks.push_front({irq_id, callback, cb_handle});
  activate_irq(irq_id);

  return true;
}

bool IRQManager::unregister_first_irq_handle(uint64_t irq_id, IRQCallBack* callback, void** callback_handle) {
  auto it = _callbacks.begin();

  while (it != std::end(_callbacks)) {
    if (it->irq == irq_id) {
      if (callback != nullptr) {
        *callback = it->cb;
      }

      if (callback_handle != nullptr) {
        *callback_handle = it->cb_handle;
      }

      _callbacks.erase(it);
      return true;
    }

    it++;
  }

  return false;
}

void IRQManager::unregister_all_irq_handle(uint64_t irq_id) {
  auto it = _callbacks.begin();

  while (it != std::end(_callbacks)) {
    if (it->irq == irq_id) {
      auto to_rem = it;
      _callbacks.erase(to_rem);
    }

    it++;
  }

  deactivate_irq(irq_id);
}

void IRQManager::activate_irq(uint64_t irq_id) {
  (*_enable_irq)(irq_id);
}

void IRQManager::deactivate_irq(uint64_t irq_id) {
  (*_disable_irq)(irq_id);
}
