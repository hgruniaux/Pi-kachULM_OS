#include "interrupts.hpp"
#include "../debug.hpp"
#include "../syscall.hpp"

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
/** Jumps from EL1 to EL0 (Non-secure). */
extern "C" void jump_from_el1_to_el0();

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

void jump_to_el0() {
  // We cannot check if we are already at EL0 (the system register CurrentEL is
  // not readable in EL0). Therefore, we assume that we are not in EL0.

  // Jump to EL1 if not already in it.
  jump_to_el1();

  // Then jump to EL0!
  jump_from_el1_to_el0();
}

extern "C" void exception_handler(InterruptSource source, InterruptKind kind, Registers& registers) {
  if (source == InterruptSource::LOWER_AARCH64 && kind == InterruptKind::SYNCHRONOUS) {
    const uint32_t ec = (registers.esr >> 26) & 0x3F;

    // Handle AArch64 syscall
    if (ec == 0b010101) {  // SVC instruction execution in AArch64 state.
      // The system call number is stored in w8 (lower 32-bits of x8).
      const uint32_t syscall_id = registers.x8 & 0xFFFFFFFF;
      SyscallManager::get().call_syscall(syscall_id, registers);
      return;
    }
  }

  if (source == InterruptSource::LOWER_AARCH32) {
    LOG_WARNING("interrupt/exception from userspace aarch32 code (not supported)");
    return;
  }

  LOG_WARNING("Unhandled exception/interrupt");
}
