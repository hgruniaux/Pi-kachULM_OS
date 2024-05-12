#include <libk/log.hpp>
#include "hardware/device.hpp"
#include "hardware/uart.hpp"

#include "hardware/kernel_dt.hpp"
#include "hardware/timer.hpp"
#include "libk/test.hpp"
#include "memory/memory.hpp"
#include "scheduler.hpp"
#include "syscall.hpp"
#include "task_manager.hpp"
#include "syscall.h"
#include "hardware/interrupts.hpp"

extern "C" void process1();
extern "C" void process2();
extern "C" void process3();

[[noreturn]] void kmain() {
  UART log(1000000);  // Set to a High Baud-rate, otherwise UART is THE bottleneck :/

  libk::register_logger(log);
  libk::set_log_timer([]() { return GenericTimer::get_elapsed_time_in_ms(); });

  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__);

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  SyscallTable* table = new SyscallTable;
  TaskManager* task_manager = new TaskManager;
  task_manager->set_default_syscall_table(table);

  Task* task1 = task_manager->create_task();
  task1->m_saved_state.regs.elr = (uint64_t)process1;
  task1->m_saved_state.regs.x30 = task1->m_saved_state.regs.elr;
  //*(char*)task1->m_saved_state.sp = 0;

  task_manager->wake_task(task1);

  table->register_syscall(SYS_PRINT, [](Registers& regs) {
    // FIXME: check if msg memory is accessible by the process
    const char* msg = (const char*)regs.x0;
    libk::print(msg);
    regs.x0 = SYS_ERR_OK;
  });

  table->register_syscall(SYS_DEBUG, [](Registers& regs) {
    Task* task = TaskManager::get().get_current_task();
    LOG_INFO("Debug syscall from pid={}", task->get_id());
    regs.x0 = SYS_ERR_OK;
  });

  table->register_syscall(SYS_EXIT, [](Registers& regs) {
    TaskManager::get().kill_current_task((int)regs.x0);
    regs.x0 = SYS_ERR_OK;
  });

  table->register_syscall(SYS_YIELD, [](Registers& regs) {
    TaskManager::get().schedule();
    regs.x0 = SYS_ERR_OK;
  });

  table->register_syscall(SYS_SPAWN, [](Registers& regs) {
    TaskManager& task_manager = TaskManager::get();
    Task* new_task = task_manager.create_task();
    if (new_task == nullptr) {
      regs.x0 = SYS_ERR_INTERNAL;
      return;
    }

    new_task->m_saved_state.regs.elr = regs.x0;
    task_manager.wake_task(new_task);
    regs.x0 = SYS_ERR_OK;
  });

  //  Enter userspace
  task_manager->schedule();
  asm volatile("mov x28, %0" : : "r"(task_manager->get_current_task()->get_saved_state().regs.x28));
  jump_to_el0(task1->get_saved_state().regs.elr, (uintptr_t)task_manager->get_current_task()->get_saved_state().sp);
  LOG_CRITICAL("Not in user space");
  libk::halt();
}
