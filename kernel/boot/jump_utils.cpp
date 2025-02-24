#include "jump_utils.hpp"

ExceptionLevel get_current_exception_level() {
  // The current exception level is stored in the system register CurrentEL in the bits [3:2].
  // https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/CurrentEL--Current-Exception-Level?lang=en
  uint64_t current_el;
  asm volatile("mrs %0, CurrentEL" : "=r"(current_el));
  return (ExceptionLevel)(current_el >> 2 & 0b11);
}

// The following functions are implemented in assembly in interrupts.S:
/** Jumps from EL3 to EL2 (Non-secure). */
extern "C" void jump_from_el3_to_el2();
/** Jumps from EL2 to EL1 (Non-secure). */
extern "C" void jump_from_el2_to_el1();

void jump_to_el1() {
  switch (get_current_exception_level()) {
    case ExceptionLevel::EL3:
      jump_from_el3_to_el2();
      [[fallthrough]];
    case ExceptionLevel::EL2:
      jump_from_el2_to_el1();
      [[fallthrough]];
    default:
      break;
  }
}
