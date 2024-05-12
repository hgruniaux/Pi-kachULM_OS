#include "hardware/device.hpp"
#include "hardware/interrupts.hpp"
#include "hardware/timer.hpp"
#include "hardware/uart.hpp"
#include "memory/memory.hpp"

#include <libk/log.hpp>
#include "boot/kernel_dt.hpp"
#include "dtb/node.hpp"
#include "scheduler.hpp"
#include "syscall.hpp"
#include "task_manager.hpp"

void print_property(const Property p) {
  if (p.is_string()) {
    LOG_INFO("DeviceTree property {} (size: {}) : '{}'", p.name, p.length, p.data);
  } else if (p.length % sizeof(uint32_t) == 0) {
    LOG_INFO("DeviceTree property {} (size: {}) :", p.name, p.length);
    const auto* data = (const uint32_t*)p.data;
    for (size_t i = 0; i < p.length / sizeof(uint32_t); i++) {
      LOG_INFO("  - At {}: {:#x}", i, libk::from_be(data[i]));
    }
  } else {
    LOG_INFO("DeviceTree property {} (size: {}) :", p.name, p.length);
    for (size_t i = 0; i < p.length; i++) {
      LOG_INFO("  - At {}: {:#x}", i, (uint8_t)p.data[i]);
    }
  }
}

void find_and_dump(libk::StringView path) {
  Property p;

  if (!KernelDT::find_property(path, &p)) {
    LOG_CRITICAL("Unable to find property: {}", path);
  }

  print_property(p);
}

void dump_current_el() {
  switch (get_current_exception_level()) {
    case ExceptionLevel::EL0:
      LOG_INFO("CurrentEL: EL0");
      break;
    case ExceptionLevel::EL1:
      LOG_INFO("CurrentEL: EL1");
      break;
    case ExceptionLevel::EL2:
      LOG_INFO("CurrentEL: EL2");
      break;
    case ExceptionLevel::EL3:
      LOG_INFO("CurrentEL: EL3");
      break;
  }
}

extern "C" void init_interrupts_vector_table();

#include "memory/mem_alloc.hpp"
#include "syscall.h"

void process2() {
  sys_print("Test 1");
  sys_yield();
  sys_print("Test 2");
  sys_yield();
  libk::halt();
}

#if 0
void process1() {
  sys_print("Foo");
  sys_spawn(process2);
  sys_yield();
  sys_print("Bar");
  sys_yield();
  libk::halt();
}
#else
extern "C" void process1();
#endif

[[noreturn]] void kmain() {
  UART u0(UART::Id::UART0, 2000000);  // Set to a High Baud rate, otherwise UART is THE bottleneck :/

  libk::register_logger(u0);

  libk::set_log_timer([]() { return GenericTimer::get_elapsed_time_in_ms(); });

  dump_current_el();
  init_interrupts_vector_table();

  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__);

  // Setup DeviceTree
  KernelMemory::init();

  // Setup Device
  if (!Device::init()) {
    libk::halt();
  }

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);
  LOG_INFO("Uart address: {:#x}", KernelDT::get_device_mmio_address("uart0"));

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
