#include "interrupts.hpp"
#include <libk/log.hpp>
#include "hardware/irq/irq_manager.hpp"
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
  auto current_task = TaskManager::get()->get_current_task();
  // current_task is guaranteed to be non-null here.

  // The system call number is stored in w8 (lower 32-bits of x8).
  const uint32_t syscall_id = registers.gp_regs.x8 & 0xFFFFFFFF;
  current_task->call_syscall(syscall_id, registers);
  return true;  // Syscall handled
}

static bool do_dispatch_userspace_interrupt(Registers& registers) {
  const uint32_t ec = (registers.esr >> 26) & 0x3F;

  auto current_task = TaskManager::get()->get_current_task();
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
  if (!do_dispatch_userspace_interrupt(registers)) {
    // Failed to handle the interrupt: probably a fatal error, kill the current task.
    TaskManager* task_manager = TaskManager::get();
    task_manager->kill_task(Task::current());
    task_manager->schedule();
  }

  return true;
}

[[noreturn]] static void dump_unhandled_interrupt(InterruptSource source, InterruptKind kind, Registers& registers) {
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

  LOG_CRITICAL("Unhandled interrupt, source = {}, kind = {}\r\nELR_EL1 = {:#x}\r\nESR_EL1 = {:#x}\r\nFAR_EL1 = {:#x}",
               source_name, kind_name, registers.elr, registers.esr, registers.far);
}

static bool do_kernelspace_interrupt(Registers& registers) {
  const uint32_t ec = (registers.esr >> 26) & 0x3F;

  switch (ec) {
      // Handle AArch64 syscall
    case 0b010101:   // SVC instruction execution in AArch64 state.
      return false;  // the syscall is handled later, once the context switcher is created
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

class ContextSwitcher {
 public:
  ContextSwitcher(Registers& regs) : m_regs(regs) { m_old_task = Task::current(); }

  ~ContextSwitcher() {
    if (m_old_task != nullptr && m_old_task->is_marked_to_be_killed())
      TaskManager::get()->kill_task(m_old_task);

    auto current_task = Task::current();
    if (current_task == m_old_task)
      return;

    if (current_task != nullptr) {
      if (m_old_task != nullptr && !m_old_task->is_terminated()) {
        m_old_task->get_saved_state().save(m_regs);
      }

      // Do context switch.
      current_task->get_saved_state().restore(m_regs);
      LOG_TRACE("Context switch to pid={} from pid={}", current_task->get_id(),
                m_old_task ? m_old_task->get_id() : UINT16_MAX);
    } else {
      LOG_CRITICAL("No more available tasks to run... The process pid=0 should never exit.");
    }
  }

 private:
  Registers& m_regs;
  TaskPtr m_old_task;
};  // class ContextSwitcher

extern "C" void exception_handler(InterruptSource source, InterruptKind kind, Registers& registers) {
  if ((source == InterruptSource::CURRENT_SP_ELX || source == InterruptSource::CURRENT_SP_EL0) &&
      kind == InterruptKind::SYNCHRONOUS) {
    if (do_kernelspace_interrupt(registers))
      return;
  }

  ContextSwitcher context_switcher(registers);

  if (kind == InterruptKind::IRQ) {
    IRQManager::handle_interrupts();
    return;
  }

  // Handle syscall from kernel code.
  const uint32_t ec = (registers.esr >> 26) & 0x3F;
  if (source == InterruptSource::CURRENT_SP_EL0 && kind == InterruptKind::SYNCHRONOUS && ec == 0b010101) {
    if (do_syscall(registers))
      return;
  }

  if (source == InterruptSource::LOWER_AARCH64 && kind == InterruptKind::SYNCHRONOUS) {
    if (do_userspace_interrupt(registers))
      return;
  }

  if (source == InterruptSource::LOWER_AARCH32) {
    LOG_WARNING("interrupt/exception from userspace aarch32 code (not supported)");
    // Kill the process.
    TaskManager* task_manager = TaskManager::get();
    task_manager->kill_task(Task::current(), 1);
    task_manager->schedule();
    return;
  }

  dump_unhandled_interrupt(source, kind, registers);
}
