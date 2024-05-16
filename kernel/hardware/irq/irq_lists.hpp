#pragma once

#include <cstddef>
#include <cstdint>
#include "irq_manager.hpp"

static inline constexpr size_t ARMC_IRQ_NB = 7;
static inline constexpr size_t VC_IRQ_NB = 64;

/** ARM Core Timer IRQ id. */
static inline constexpr IRQ ARMC_TIMER = {.type = IRQ::Type::ARMCore, .id = 0};

/** ARM Core Mailbox IRQ id. */
static inline constexpr IRQ ARMC_MAILBOX = {.type = IRQ::Type::ARMCore, .id = 1};

/** ARM Core Doorbell0 IRQ id. */
static inline constexpr IRQ ARMC_DOORBELL0 = {.type = IRQ::Type::ARMCore, .id = 2};

/** ARM Core Doorbell1 IRQ id. */
static inline constexpr IRQ ARMC_DOORBELL1 = {.type = IRQ::Type::ARMCore, .id = 3};

/** ARM Core GPU0 halted IRQ id. */
static inline constexpr IRQ ARMC_GPU0_HALTED = {.type = IRQ::Type::ARMCore, .id = 4};

/** ARM Core GPU1 halted IRQ id. */
static inline constexpr IRQ ARMC_GPU1_HALTED = {.type = IRQ::Type::ARMCore, .id = 5};

/** ARM Core Illegal access type 1 IRQ id. */
static inline constexpr IRQ ARMC_ILLEGAL_ACCESS_TYPE1 = {.type = IRQ::Type::ARMCore, .id = 6};

/** ARM Core Illegal access type 0 IRQ id. */
static inline constexpr IRQ ARMC_ILLEGAL_ACCESS_TYPE0 = {.type = IRQ::Type::ARMCore, .id = 7};

/** Base for Timer IRQ id. */
static inline constexpr IRQ VC_TIMER_BASE = {.type = IRQ::Type::VideoCore, .id = 0};
// Timer 0: 0
// Timer 1: 1
// Timer 2: 2
// Timer 3: 3

/** AUX IRQ id. */
static inline constexpr IRQ VC_AUX = {.type = IRQ::Type::VideoCore, .id = 29};

/** Base for GPIO IRQ id. */
static inline constexpr IRQ VC_GPIO_BASE = {.type = IRQ::Type::VideoCore, .id = 49};
// GPIO 0: 49
// GPIO 1: 50
// GPIO 2: 51
// GPIO 3: 52

/** I2C IRQ id. */
static inline constexpr IRQ VC_I2C = {.type = IRQ::Type::VideoCore, .id = 53};

/** SPI IRQ id. */
static inline constexpr IRQ VC_SPI = {.type = IRQ::Type::VideoCore, .id = 54};

/** PCM IRQ id. */
static inline constexpr IRQ VC_PCM = {.type = IRQ::Type::VideoCore, .id = 55};

/** UART IRQ id. */
static inline constexpr IRQ VC_UART = {.type = IRQ::Type::VideoCore, .id = 57};

/** EMMC IRQ id. */
static inline constexpr IRQ VC_EMMC = {.type = IRQ::Type::VideoCore, .id = 62};
