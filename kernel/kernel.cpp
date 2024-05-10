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

void user_space_init() {
  asm volatile("mov w8, 3\n\tsvc #0");  // schedule
}

void sys_yield() {
  asm volatile("mov w8, 3\n\tsvc #0" : : : "w8");
}

void sys_print(const char* str) {
  asm volatile("mov w8, 1\n\tmov x0, %0\n\tsvc #0" : : "r"(str) : "w8", "x0");
}

void process1() {
  sys_print("Foo");
  sys_yield();
  sys_print("Bar");
  sys_yield();
  libk::halt();
}

void process2() {
  sys_print("Test 1");
  sys_yield();
  sys_print("Test 2");
  sys_yield();
  libk::halt();
}

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

  Scheduler* scheduler = new Scheduler;
  SyscallTable* table = new SyscallTable;

  Task* task1 = new Task;
  task1->m_id = 1;
  task1->m_saved_state.regs.elr = (uint64_t)process1;
  task1->m_syscall_table = table;
  scheduler->add_task(task1);

  Task* task2 = new Task;
  task2->m_id = 2;
  task2->m_saved_state.regs.elr = (uint64_t)process2;
  task2->m_syscall_table = table;
  scheduler->add_task(task2);

  table->register_syscall(1, [](Registers& regs) {
    LOG_INFO("Print syscall");
    // FIXME: check if msg memory is accessible by the process
    const char* msg = (const char*)regs.x0;
    libk::print(msg);
    LOG_INFO("Current process {}", Scheduler::get().get_current_task()->get_id());
  });

  table->register_syscall(2, [](Registers&) { LOG_INFO("Exit syscall"); });

  table->register_syscall(3, [](Registers&) {
    if (Scheduler::get().get_current_task() != nullptr) {
      LOG_INFO("yield from pid={}", Scheduler::get().get_current_task()->get_id());
    }
    Scheduler::get().schedule();
  });

  //  Enter userspace
  scheduler->schedule();
  jump_to_el0(task1->get_saved_state().regs.elr);
  LOG_CRITICAL("Not in user space");
  libk::halt();
}
