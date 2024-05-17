#include "bcm2711_irq_manager.hpp"
#include "hardware/kernel_dt.hpp"
#include "irq_lists.hpp"
#include "libk/log.hpp"

static inline constexpr uint32_t GICD_BASE = 0;
static inline constexpr uint32_t GICC_BASE = 0x1000;

static inline constexpr uint32_t GICD_ISENABLER_BASE = GICD_BASE + 0x100;
static inline constexpr uint32_t GICD_ICENABLER_BASE = GICD_BASE + 0x180;
static inline constexpr uint32_t GICD_ITARGETSR_BASE = GICD_BASE + 0x800;

static inline constexpr uint32_t GICC_IAR = GICC_BASE + 0x0C;
static inline constexpr uint32_t GICC_EOIR = GICC_BASE + 0x10;

static inline constexpr uint32_t ARMC_IRQ_START = 64;
static inline constexpr uint32_t VC_IRQ_START = 96;

static uintptr_t _base;

void enable_irq_gid_range(uint32_t irq_gic_start, uint32_t irq_gid_stop) {
  uint32_t core_id;
  asm volatile("mrs %x0, mpidr_el1" : "=r"(core_id));
  core_id &= 0b11;

  for (uint32_t irq_gic_id = irq_gic_start; irq_gic_id < irq_gid_stop; ++irq_gic_id) {
    const uint32_t n = irq_gic_id / 4;
    const uint32_t byte_offset = irq_gic_id % 4;
    const uintptr_t reg_address = _base + GICD_ITARGETSR_BASE + n * sizeof(uint32_t);
    const uint32_t shift = byte_offset * 8 + core_id;

    const uint32_t prev_val = libk::read32(reg_address);
    libk::write32(reg_address, prev_val | (1 << shift));
  }
}

void BCM2711_IRQManager::init() {
  _base = KernelDT::force_get_device_address("gicv2");

  enable_irq_gid_range(ARMC_IRQ_START, ARMC_IRQ_START + ARMC_IRQ_NB);
  enable_irq_gid_range(VC_IRQ_START, VC_IRQ_START + VC_IRQ_NB);
}

void enable_gic_distributor(uint32_t irq_id) {
  const uint32_t n = irq_id / 32;
  const uint32_t shift = irq_id % 32;
  libk::write32(_base + GICD_ISENABLER_BASE + n * sizeof(uint32_t), (1 << shift));
}

void disable_gic_distributor(uint32_t irq_id) {
  const uint32_t n = irq_id / 32;
  const uint32_t shift = irq_id % 32;
  libk::write32(_base + GICD_ICENABLER_BASE + n * sizeof(uint32_t), (1 << shift));
}

void BCM2711_IRQManager::enable_irq(IRQ irq) {
  switch (irq.type) {
    case IRQ::Type::ARMCore:
      enable_gic_distributor(irq.id + ARMC_IRQ_START);
      break;

    case IRQ::Type::VideoCore:
      enable_gic_distributor(irq.id + VC_IRQ_START);
      break;
  }
}

void BCM2711_IRQManager::disable_irq(IRQ irq) {
  switch (irq.type) {
    case IRQ::Type::ARMCore:
      disable_gic_distributor(irq.id + ARMC_IRQ_START);
      break;

    case IRQ::Type::VideoCore:
      disable_gic_distributor(irq.id + VC_IRQ_START);
      break;
  }
}

void BCM2711_IRQManager::mask_as_processed(IRQ irq) {
  switch (irq.type) {
    case IRQ::Type::ARMCore:
      libk::write32(_base + GICC_EOIR, irq.id + ARMC_IRQ_START);
      break;

    case IRQ::Type::VideoCore:
      libk::write32(_base + GICC_EOIR, irq.id + VC_IRQ_START);
      break;
  }
}

bool BCM2711_IRQManager::has_pending_interrupt(IRQ* irq) {
  const uint32_t IAR = libk::read32(_base + GICC_IAR);
  const uint64_t irq_gic_id = IAR & libk::mask_bits(0, 23);

  if (ARMC_IRQ_START <= irq_gic_id && irq_gic_id < ARMC_IRQ_START + ARMC_IRQ_NB) {
    irq->type = IRQ::Type::ARMCore;
    irq->id = irq_gic_id - ARMC_IRQ_START;
    return true;
  }

  if (VC_IRQ_START <= irq_gic_id && irq_gic_id < VC_IRQ_START + VC_IRQ_NB) {
    irq->type = IRQ::Type::VideoCore;
    irq->id = irq_gic_id - VC_IRQ_START;
    return true;
  }

  return false;
}
