#include "bcm2837_irq_manager.hpp"
#include "hardware/kernel_dt.hpp"

#include <libk/log.hpp>
#include "hardware/irq/irq_lists.hpp"

#define FILL_IRQ(irq_handler, var, bit, irq_id, irq_type) \
  {                                                       \
    if ((((var) >> (bit)) & 0b1) != 0) {                  \
      irq_handler->type = (irq_type);                     \
      irq_handler->id = (irq_id);                         \
                                                          \
      return true;                                        \
    }                                                     \
  }

static inline constexpr uint32_t IRQ_PEND_BASIC = 0x0;  // IRQ basic pending
static inline constexpr uint32_t IRQ_PEND_1 = 0x04;     // IRQ pending 1
static inline constexpr uint32_t IRQ_PEND_2 = 0x08;     // IRQ pending 2

// static inline constexpr uint32_t FIQ_CONTROL = 0x0C;    // FIQ control

/** Video Core IRQ Enable Base */
static inline constexpr uint32_t IRQ_ENABLE_VC_BASE = 0x10;
// IRQ_ENABLE_1: 0x10
// IRQ_ENABLE_2: 0x14

/** ARM Enable IRQ */
static inline constexpr uint32_t IRQ_ENABLE_BASIC = 0x18;

/** Video Core IRQ Disable Base */
static inline constexpr uint32_t IRQ_DISABLE_VC_BASE = 0x1C;
// IRQ_DISABLE_1: 0x1C
// IRQ_DISABLE_2: 0x20

/** ARM Disable IRQ */
static inline constexpr uint32_t IRQ_DISABLE_BASIC = 0x24;

static uintptr_t _base;

void BCM2837_IRQManager::init() {
  _base = KernelDT::force_get_device_address("intc");
}

void BCM2837_IRQManager::enable_irq(IRQ irq) {
  switch (irq.type) {
    case IRQ::Type::ARMCore: {
      if (irq.id > 7) {
        LOG_ERROR("Unknown ARM IRQ {}", irq.id);
        libk::panic("Unable to activate an IRQ");
      }

      const uint32_t enable_mask = (uint32_t)1 << irq.id;
      libk::write32(_base + IRQ_ENABLE_BASIC, enable_mask);

      break;
    }
    case IRQ::Type::VideoCore: {
      const uint8_t enable_reg = irq.id / 32;

      if (enable_reg > 1) {
        LOG_ERROR("Unknown VideoCore IRQ {}", irq.id);
        libk::panic("Unable to activate an IRQ");
      }

      const uint32_t enable_mask = (uint32_t)1 << (irq.id % 32);
      libk::write32(_base + IRQ_ENABLE_VC_BASE + enable_reg * sizeof(uint32_t), enable_mask);

      break;
    }
  }
}

void BCM2837_IRQManager::disable_irq(IRQ irq) {
  switch (irq.type) {
    case IRQ::Type::ARMCore: {
      if (irq.id > 7) {
        LOG_ERROR("Unknown ARM IRQ {}", irq.id);
        libk::panic("Unable to activate an IRQ");
      }

      const uint32_t disable_mask = (uint32_t)1 << irq.id;
      libk::write32(_base + IRQ_DISABLE_BASIC, disable_mask);

      break;
    }
    case IRQ::Type::VideoCore: {
      const uint8_t disable_reg = irq.id / 32;

      if (disable_reg > 1) {
        LOG_ERROR("Unknown VideoCore IRQ {}", irq.id);
        libk::panic("Unable to activate an IRQ");
      }

      const uint32_t disable_mask = (uint32_t)1 << (irq.id % 32);
      libk::write32(_base + IRQ_DISABLE_VC_BASE + disable_reg * sizeof(uint32_t), disable_mask);

      break;
    }
  }
}

void BCM2837_IRQManager::mask_as_processed(IRQ irq) {
  (void)irq;
}

bool fill_vc_1(IRQ* irq) {
  const uint32_t vc_1_pending = libk::read32(_base + IRQ_PEND_1);

  FILL_IRQ(irq, vc_1_pending, 0, VC_TIMER_BASE.id + 0, VC_TIMER_BASE.type);
  FILL_IRQ(irq, vc_1_pending, 1, VC_TIMER_BASE.id + 1, VC_TIMER_BASE.type);
  FILL_IRQ(irq, vc_1_pending, 2, VC_TIMER_BASE.id + 2, VC_TIMER_BASE.type);
  FILL_IRQ(irq, vc_1_pending, 3, VC_TIMER_BASE.id + 3, VC_TIMER_BASE.type);

  FILL_IRQ(irq, vc_1_pending, 29, VC_AUX.id, VC_AUX.type);

  return false;
}

bool fill_vc_2(IRQ* irq) {
  const uint32_t vc_2_pending = libk::read32(_base + IRQ_PEND_2);

  FILL_IRQ(irq, vc_2_pending, 17 /* 49 */, VC_GPIO_BASE.id + 0, VC_GPIO_BASE.type);
  FILL_IRQ(irq, vc_2_pending, 18 /* 50 */, VC_GPIO_BASE.id + 1, VC_GPIO_BASE.type);
  FILL_IRQ(irq, vc_2_pending, 19 /* 51 */, VC_GPIO_BASE.id + 2, VC_GPIO_BASE.type);
  FILL_IRQ(irq, vc_2_pending, 20 /* 52 */, VC_GPIO_BASE.id + 3, VC_GPIO_BASE.type);

  FILL_IRQ(irq, vc_2_pending, 21 /* 53 */, VC_I2C.id, VC_I2C.type);
  FILL_IRQ(irq, vc_2_pending, 22 /* 54 */, VC_SPI.id, VC_SPI.type);
  FILL_IRQ(irq, vc_2_pending, 23 /* 55 */, VC_PCM.id, VC_PCM.type);

  FILL_IRQ(irq, vc_2_pending, 25 /* 57 */, VC_UART.id, VC_UART.type);

  FILL_IRQ(irq, vc_2_pending, 30 /* 62 */, VC_EMMC.id, VC_EMMC.type);

  return false;
}

bool BCM2837_IRQManager::has_pending_interrupt(IRQ* irq) {
  const uint32_t base_pending = libk::read32(_base + IRQ_PEND_BASIC);

  FILL_IRQ(irq, base_pending, 0, ARMC_TIMER.id, ARMC_TIMER.type);
  FILL_IRQ(irq, base_pending, 1, ARMC_MAILBOX.id, ARMC_MAILBOX.type);
  FILL_IRQ(irq, base_pending, 2, ARMC_DOORBELL0.id, ARMC_DOORBELL0.type);
  FILL_IRQ(irq, base_pending, 3, ARMC_DOORBELL1.id, ARMC_DOORBELL1.type);
  FILL_IRQ(irq, base_pending, 4, ARMC_GPU0_HALTED.id, ARMC_GPU0_HALTED.type);
  FILL_IRQ(irq, base_pending, 5, ARMC_GPU1_HALTED.id, ARMC_GPU1_HALTED.type);
  FILL_IRQ(irq, base_pending, 6, ARMC_ILLEGAL_ACCESS_TYPE1.id, ARMC_ILLEGAL_ACCESS_TYPE1.type);
  FILL_IRQ(irq, base_pending, 7, ARMC_ILLEGAL_ACCESS_TYPE0.id, ARMC_ILLEGAL_ACCESS_TYPE0.type);

  FILL_IRQ(irq, base_pending, 15, VC_I2C.id, VC_I2C.type);
  FILL_IRQ(irq, base_pending, 16, VC_SPI.id, VC_SPI.type);
  FILL_IRQ(irq, base_pending, 17, VC_PCM.id, VC_PCM.type);
  FILL_IRQ(irq, base_pending, 19, VC_UART.id, VC_UART.type);
  FILL_IRQ(irq, base_pending, 20, VC_EMMC.id, VC_EMMC.type);

  if (((base_pending >> 8) & 0b1) != 0 && fill_vc_1(irq)) {
    return true;
  }

  if (((base_pending >> 9) & 0b1) != 0 && fill_vc_2(irq)) {
    return true;
  }

  return false;
}
