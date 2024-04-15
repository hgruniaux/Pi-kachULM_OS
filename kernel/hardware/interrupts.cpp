#include "interrupts.hpp"
#include <libk/log.hpp>
#include "../syscall.hpp"

#define dump_reg(reg)                           \
  {                                             \
    uint64_t tmp;                               \
    asm volatile("mrs %x0, " #reg : "=r"(tmp)); \
    LOG_INFO("Register " #reg ": {}", tmp);     \
  }

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

  dump_reg(ELR_EL1);
  dump_reg(ESR_EL1);
  dump_reg(SPSR_EL1);
  dump_reg(SCTLR_EL1);
  dump_reg(TCR_EL1);
  dump_reg(MAIR_EL1);
  dump_reg(TTBR1_EL1);
  dump_reg(SCTLR_EL1);

  LOG_CRITICAL("Unhandled exception/interrupt");
}
