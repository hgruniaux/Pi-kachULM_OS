#include "bcm2837_irq_manager.hpp"
#include "hardware/kernel_dt.hpp"

#include <libk/log.hpp>
#include "hardware/irq/irq_lists.hpp"

#define IF_BIT_SET(irq_handler, var, bit, irq) \
  {                                            \
    if ((((var) >> (bit)) & 0b1) != 0) {       \
      *(irq_handler) = (irq);                  \
      return true;                             \
    }                                          \
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

void BCM2837_IRQManager::enable_irq(uint32_t irq_id) {
  const bool is_vc_irq = (irq_id & VC_IRQ_MASK) != 0;

  if (is_vc_irq) {
    const uint64_t vc_irq = irq_id & ~(VC_IRQ_MASK);
    const uint8_t enable_reg = vc_irq / 32;

    if (enable_reg > 1) {
      LOG_ERROR("Unknown VideoCore IRQ {}", vc_irq);
      libk::panic("Unable to activate an IRQ");
    }

    const uint32_t enable_mask = (uint32_t)1 << (vc_irq % 32);
    libk::write32(_base + IRQ_ENABLE_VC_BASE + enable_reg * sizeof(uint32_t), enable_mask);
    return;
  }

  const bool is_armc_irq = (irq_id & ARMC_IRQ_MASK) != 0;

  if (is_armc_irq) {
    // It's an ARM IRQ.
    const uint64_t arm_irq = irq_id & ~(ARMC_IRQ_MASK);
    if (arm_irq > 7) {
      LOG_ERROR("Unknown ARM IRQ {}", arm_irq);
      libk::panic("Unable to activate an IRQ");
    }

    const uint32_t enable_mask = (uint32_t)1 << arm_irq;
    libk::write32(_base + IRQ_ENABLE_BASIC, enable_mask);
  }
}

void BCM2837_IRQManager::disable_irq(uint32_t irq_id) {
  const bool is_vc_irq = (irq_id & VC_IRQ_MASK) != 0;

  if (is_vc_irq) {
    const uint64_t vc_irq = irq_id & ~(VC_IRQ_MASK);
    const uint8_t enable_reg = vc_irq / 32;

    if (enable_reg > 1) {
      LOG_ERROR("Unknown VideoCore IRQ {}", vc_irq);
      libk::panic("Unable to deactivate an IRQ");
    }

    const uint32_t disable_mask = (uint32_t)1 << (vc_irq % 32);
    libk::write32(_base + IRQ_DISABLE_VC_BASE + enable_reg * sizeof(uint32_t), disable_mask);
    return;
  }

  const bool is_armc_irq = (irq_id & ARMC_IRQ_MASK) != 0;

  if (is_armc_irq) {
    // It's an ARM IRQ.
    const uint64_t arm_irq = irq_id & ~(ARMC_IRQ_MASK);
    if (arm_irq > 7) {
      LOG_ERROR("Unknown ARM IRQ {}", arm_irq);
      libk::panic("Unable to deactivate an IRQ");
    }

    const uint32_t disable_mask = (uint32_t)1 << arm_irq;
    libk::write32(_base + IRQ_DISABLE_BASIC, disable_mask);
  }
}

void BCM2837_IRQManager::mask_as_processed(uint32_t irq_id) {
  (void)irq_id;
}

bool fill_vc_1(uint32_t* irq_id) {
  const uint32_t vc_1_pending = libk::read32(_base + IRQ_PEND_1);

  IF_BIT_SET(irq_id, vc_1_pending, 0, VC_TIMER_BASE + 0);
  IF_BIT_SET(irq_id, vc_1_pending, 1, VC_TIMER_BASE + 1);
  IF_BIT_SET(irq_id, vc_1_pending, 2, VC_TIMER_BASE + 2);
  IF_BIT_SET(irq_id, vc_1_pending, 3, VC_TIMER_BASE + 3);

  IF_BIT_SET(irq_id, vc_1_pending, 29, VC_AUX);

  return false;
}

bool fill_vc_2(uint32_t* irq_id) {
  const uint32_t vc_2_pending = libk::read32(_base + IRQ_PEND_2);

  IF_BIT_SET(irq_id, vc_2_pending, 17 /* 49 */, VC_GPIO_BASE + 0);
  IF_BIT_SET(irq_id, vc_2_pending, 18 /* 50 */, VC_GPIO_BASE + 1);
  IF_BIT_SET(irq_id, vc_2_pending, 19 /* 51 */, VC_GPIO_BASE + 2);
  IF_BIT_SET(irq_id, vc_2_pending, 20 /* 52 */, VC_GPIO_BASE + 3);

  IF_BIT_SET(irq_id, vc_2_pending, 21 /* 53 */, VC_I2C);
  IF_BIT_SET(irq_id, vc_2_pending, 22 /* 54 */, VC_SPI);
  IF_BIT_SET(irq_id, vc_2_pending, 23 /* 55 */, VC_PCM);

  IF_BIT_SET(irq_id, vc_2_pending, 25 /* 57 */, VC_UART);

  IF_BIT_SET(irq_id, vc_2_pending, 30 /* 62 */, VC_EMMC);

  return false;
}

bool BCM2837_IRQManager::has_pending_interrupt(uint32_t* irq_id) {
  const uint32_t base_pending = libk::read32(_base + IRQ_PEND_BASIC);

  IF_BIT_SET(irq_id, base_pending, 0, ARMC_TIMER);
  IF_BIT_SET(irq_id, base_pending, 1, ARMC_MAILBOX);
  IF_BIT_SET(irq_id, base_pending, 2, ARMC_DOORBELL0);
  IF_BIT_SET(irq_id, base_pending, 3, ARMC_DOORBELL1);
  IF_BIT_SET(irq_id, base_pending, 4, ARMC_GPU0_HALTED);
  IF_BIT_SET(irq_id, base_pending, 5, ARMC_GPU1_HALTED);
  IF_BIT_SET(irq_id, base_pending, 6, ARMC_ILLEGAL_ACCESS_TYPE1);
  IF_BIT_SET(irq_id, base_pending, 7, ARMC_ILLEGAL_ACCESS_TYPE0);

  IF_BIT_SET(irq_id, base_pending, 15, VC_I2C);
  IF_BIT_SET(irq_id, base_pending, 16, VC_SPI);
  IF_BIT_SET(irq_id, base_pending, 17, VC_PCM);
  IF_BIT_SET(irq_id, base_pending, 19, VC_UART);
  IF_BIT_SET(irq_id, base_pending, 20, VC_EMMC);

  if (((base_pending >> 8) & 0b1) != 0 && fill_vc_1(irq_id)) {
    return true;
  }

  if (((base_pending >> 9) & 0b1) != 0 && fill_vc_2(irq_id)) {
    return true;
  }

  return false;
}
