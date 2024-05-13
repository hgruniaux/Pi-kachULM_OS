#include "interrupts.hpp"
#include <libk/log.hpp>
#include "task/task_manager.hpp"

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

static bool do_syscall(Registers& registers) {
  Task* current_task = TaskManager::get().get_current_task();
  // current_task is guaranteed to be non-null here.

  // The system call number is stored in w8 (lower 32-bits of x8).
  const uint32_t syscall_id = registers.x8 & 0xFFFFFFFF;
  current_task->call_syscall(syscall_id, registers);
  return true;  // Syscall handled
}

static bool do_dispatch_userspace_interrupt(Registers& registers) {
  const uint32_t ec = (registers.esr >> 26) & 0x3F;

  Task* current_task = TaskManager::get().get_current_task();
  // current_task is guaranteed to be non-null here.

  const auto pid = current_task->get_id();
  const auto far = registers.far;
  const auto pc = registers.elr;
  switch (ec) {
      // Handle AArch64 syscall
    case 0b010101:  // SVC instruction execution in AArch64 state.
      return do_syscall(registers);
    case 0b100000:
      LOG_WARNING("Instruction Abort from user space (pid={}) at {:#x}. PC = {:#x}", pid, far, pc);
      break;
    case 0b100100:
      LOG_WARNING("Data Abort from user space (pid={}) at {:#x}. PC = {:#x}", pid, far, pc);
      break;
    case 0b100010:
      LOG_WARNING("PC alignment fault exception from user space (pid={}) at {:#x}. PC = {:#x}", pid, far, pc);
      break;
    case 0b100110:
      LOG_WARNING("SP alignment fault exception from user space (pid={}) at {:#x}. PC = {:#x}", pid, far, pc);
      break;
    case 0b101100:
      LOG_WARNING("Trapped floating-point exception from user space (pid={}). PC = {:#x}", pid, pc);
      break;
    default:
      LOG_WARNING("Exception with EC={:#b} from user space (pid={}). PC = {:#x}", ec, pid, pc);
      break;
  }

  return false;
}

static bool do_userspace_interrupt(Registers& registers) {
  TaskManager& task_manager = TaskManager::get();

  Task* current_task = task_manager.get_current_task();
  KASSERT(current_task != nullptr);

  current_task->get_saved_state().save(registers);

  if (!do_dispatch_userspace_interrupt(registers)) {
    // Failed to handle the interrupt: probably a fatal error, kill the current task.
    current_task = task_manager.get_current_task();
    KASSERT(current_task != nullptr);
    task_manager.kill_task(current_task);
  }

  current_task = task_manager.get_current_task();
  if (current_task != nullptr) {
    // Do context switch.
    current_task->get_saved_state().restore(registers);
  } else {
    LOG_WARNING("No more available tasks... the kernel will enter an infinite loop.");
    libk::halt();
  }

  return true;
}

static bool do_kernelspace_interrupt(Registers& registers) {
  const uint32_t ec = (registers.esr >> 26) & 0x3F;

  switch (ec) {
    case 0b100001:
      LOG_WARNING("Instruction Abort from kernel space at {:#x}.", registers.far);
      break;
    case 0b100101:
      LOG_WARNING("Data Abort from kernel space at {:#x}.", registers.far);
      break;
    case 0b100010:
      LOG_WARNING("PC alignment fault exception from kernel space at {:#x}.", registers.far);
      break;
    case 0b100110:
      LOG_WARNING("SP alignment fault exception from kernel space at {:#x}.", registers.far);
      break;
    case 0b101100:
      LOG_WARNING("Trapped floating-point exception from kernel space.");
      break;
    default:
      LOG_WARNING("Exception with EC={:#b} from kernel space.", ec);
      break;
  }

  return false;
}

static void dump_unhandled_interrupt(InterruptSource source, InterruptKind kind, Registers& registers) {
  const char* source_name = nullptr;
  switch (source) {
    case InterruptSource::CURRENT_SP_EL0:
      source_name = "CURRENT_SP_EL0";
      break;
    case InterruptSource::CURRENT_SP_ELX:
      source_name = "CURRENT_SP_ELX";
      break;
    case InterruptSource::LOWER_AARCH32:
      source_name = "LOWER_AARCH32";
      break;
    case InterruptSource::LOWER_AARCH64:
      source_name = "LOWER_AARCH64";
      break;
  }

  const char* kind_name = nullptr;
  switch (kind) {
    case InterruptKind::SYNCHRONOUS:
      kind_name = "SYNCHRONOUS";
      break;
    case InterruptKind::IRQ:
      kind_name = "IRQ";
      break;
    case InterruptKind::FIQ:
      kind_name = "FIQ";
      break;
    case InterruptKind::SERROR:
      kind_name = "SERROR";
      break;
  }

  LOG_CRITICAL("Unhandled interrupt, source = {}, kind = {}\nELR_EL1 = {:#x}\nESR_EL1 = {:#x}\nFAR_EL1 = {:#x}",
               source_name, kind_name, registers.elr, registers.esr, registers.far);
}

extern "C" void exception_handler(InterruptSource source, InterruptKind kind, Registers& registers) {
  if (source == InterruptSource::LOWER_AARCH64 && kind == InterruptKind::SYNCHRONOUS) {
    if (do_userspace_interrupt(registers))
      return;
  }

  if (source == InterruptSource::LOWER_AARCH32) {
    LOG_WARNING("interrupt/exception from userspace aarch32 code (not supported)");
    // FIXME: Crash the userspace task.
    return;
  }

  if (source == InterruptSource::CURRENT_SP_ELX && kind == InterruptKind::SYNCHRONOUS) {
    if (do_kernelspace_interrupt(registers))
      return;
  }

  dump_unhandled_interrupt(source, kind, registers);
}
