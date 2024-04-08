#pragma once

#include <cstdint>

enum class ExceptionLevel : uint8_t {
  EL0 = 0,
  EL1 = 1,
  EL2 = 2,
  EL3 = 3,
};  // enum class ExceptionLevel

[[nodiscard]] inline bool operator<(ExceptionLevel lhs, ExceptionLevel rhs) {
  return (uint8_t)lhs < (uint8_t)rhs;
}

/**
 * Returns the current exception level of the CPU (either EL0, EL1, EL2 or EL3).
 *
 * @warning This function cannot be called from EL0.
 */
[[nodiscard]] ExceptionLevel get_current_exception_level();

/** Enables the FPU and Neon unit. To be called when in EL3 or EL2. */
extern "C" void enable_fpu_and_neon();

/** Jumps from EL3 to EL2 (Non-secure). */
extern "C" void jump_from_el3_to_el2();
/** Jumps from EL2 to EL1 (Non-secure). */
extern "C" void jump_from_el2_to_el1();
/** Jumps from EL1 to EL0 (Non-secure). */
extern "C" void jump_from_el1_to_el0();

/**
 * This function jumps the kernel from its current exception level to the EL1.
 *
 * This works even if the CPU is initially in EL3 or EL2. If the CPU is already
 * in EL1, then nothing is done.  However, if the CPU is in EL0, the kernel will crash.
 */
void jump_to_el1();
