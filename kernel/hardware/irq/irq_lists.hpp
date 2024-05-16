#pragma once

#include <cstdint>

static inline constexpr uint64_t VC_IRQ_MASK = 0x1000;
static inline constexpr uint64_t ARM_IRQ_MASK = 0x0000;

/** ARM Timer IRQ id. */
static inline constexpr uint64_t ARM_TIMER = ARM_IRQ_MASK | 0;

/** ARM Mailbox IRQ id. */
static inline constexpr uint64_t ARM_MAILBOX = ARM_IRQ_MASK | 1;

/** ARM Doorbell0 IRQ id. */
// static inline constexpr uint64_t ARM_DOORBELL0 = ARM_IRQ_MASK | 2;

/** ARM Doorbell1 IRQ id. */
// static inline constexpr uint64_t ARM_DOORBELL1 = ARM_IRQ_MASK | 3;

/** ARM GPU0 halted IRQ id. */
// static inline constexpr uint64_t ARM_GPU0_HALTED = ARM_IRQ_MASK | 4;

/** ARM GPU1 halted IRQ id. */
// static inline constexpr uint64_t ARM_GPU1_HALTED = ARM_IRQ_MASK | 5;

/** ARM Illegal access type 1 IRQ id. */
// static inline constexpr uint64_t ARM_ILLEGAL_ACCESS_TYPE1 = ARM_IRQ_MASK | 6;

/** ARM Illegal access type 0 IRQ id. */
// static inline constexpr uint64_t ARM_ILLEGAL_ACCESS_TYPE0 = ARM_IRQ_MASK | 7;

/** Base for Timer IRQ id. */
static inline constexpr uint64_t VC_TIMER_BASE = VC_IRQ_MASK | 0;
// Timer 0: 0
// Timer 1: 1
// Timer 2: 2
// Timer 3: 3

/** AUX IRQ id. */
static inline constexpr uint64_t VC_AUX = VC_IRQ_MASK | 29;

/** Base for GPIO IRQ id. */
static inline constexpr uint64_t VC_GPIO_BASE = VC_IRQ_MASK | 49;
// GPIO 0: 49
// GPIO 1: 50
// GPIO 2: 51
// GPIO 3: 52

/** I2C IRQ id. */
static inline constexpr uint64_t VC_I2C = VC_IRQ_MASK | 53;

/** SPI IRQ id. */
static inline constexpr uint64_t VC_SPI = VC_IRQ_MASK | 54;

/** PCM IRQ id. */
static inline constexpr uint64_t VC_PCM = VC_IRQ_MASK | 55;

/** UART IRQ id. */
static inline constexpr uint64_t VC_UART = VC_IRQ_MASK | 57;

/** EMMC IRQ id. */
static inline constexpr uint64_t VC_EMMC = VC_IRQ_MASK | 62;
