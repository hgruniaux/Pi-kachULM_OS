#pragma once

#include <cstdint>

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

enum class ExceptionLevel : uint8_t {
  /** The user space level. The least privileged level. */
  EL0 = 0,
  /** The kernel level. */
  EL1 = 1,
  /** The hypervisor level. */
  EL2 = 2,
  /** The secure monitor level. The most privileged level. */
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

/**
 * This function jumps the kernel from its current exception level to the EL1.
 *
 * This works even if the CPU is initially in EL3 or EL2. If the CPU is already
 * in EL1, then nothing is done.  However, if the CPU is in EL0, the kernel will crash.
 */
extern "C" void jump_to_el1();

/**
 * This function jumps the kernel from EL1 to EL0.
 *
 * @warning It is assumed that the kernel is not yet in EL0. The kernel will
 * crash if the assumption is not hold.
 */
extern "C" void jump_to_el0(uintptr_t elr, uintptr_t stack);

void disable_interrupts();
void enable_interrupts();

class DisableInterrupts {
 public:
  DisableInterrupts() { disable_interrupts(); }
  ~DisableInterrupts() { enable_interrupts(); }
};  // class DisableInterrupts
