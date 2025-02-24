#pragma once

#include <cstdint>

#include "boot/jump_utils.hpp"

enum class InterruptSource : uint8_t {
  /** The interrupt comes from the same exception level and the stack pointer
   * is stored in SP_EL0. Currently, this should never happen as the CPU is
   * configured by the kernel to use SP_ELx for each level. */
  CURRENT_SP_EL0 = 0,
  /** The interrupt comes from the same exception level and the stack pointer
   * is stored in SP_ELx (stack pointer per exception level). */
  CURRENT_SP_ELX = 1,
  /** The interrupts comes from a lower exception level in AArch64 mode. */
  LOWER_AARCH64 = 2,
  /** The interrupts comes from a lower exception level in AArch32 mode. */
  LOWER_AARCH32 = 3,
};  // enum class InterruptSource

enum class InterruptKind : uint8_t {
  SYNCHRONOUS = 0,
  IRQ = 1,
  FIQ = 2,
  SERROR = 3,
};  // enum class InterruptKind
