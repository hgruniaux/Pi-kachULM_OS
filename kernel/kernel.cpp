#include <libk/log.hpp>
#include "hardware/device.hpp"
#include "hardware/uart.hpp"

#include "hardware/interrupts.hpp"
#include "hardware/kernel_dt.hpp"
#include "hardware/timer.hpp"
#include "task/task_manager.hpp"

#include <syscall/syscall.h>

extern "C" const char init[];

[[noreturn]] void kmain() {
  UART log(1000000);  // Set to a High Baud-rate, otherwise UART is THE bottleneck :/

  libk::register_logger(log);
  libk::set_log_timer([]() { return GenericTimer::get_elapsed_time_in_ms(); });

  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__);

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  TaskManager* task_manager = new TaskManager;

  auto task1 = task_manager->create_task((const elf::Header*)&init);
  task_manager->wake_task(task1.get());

  //  Enter userspace
  task_manager->schedule();
  task_manager->get_current_task()->get_saved_state().memory->activate();
  jump_to_el0(task1->get_saved_state().regs.elr, (uintptr_t)task_manager->get_current_task()->get_saved_state().sp);
  LOG_CRITICAL("Not in user space");
  libk::halt();
}
