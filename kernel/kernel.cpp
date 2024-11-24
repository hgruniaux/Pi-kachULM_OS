#include <libk/log.hpp>

#include "hardware/device.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/interrupts.hpp"
#include "hardware/kernel_dt.hpp"
#include "hardware/ps2_keyboard.hpp"
#include "hardware/uart_keyboard.hpp"

#include "fs/filesystem.hpp"

#include "sys/syscall.h"
#include "task/task_manager.hpp"
#include "wm/window_manager.hpp"

#if defined(__clang__)
#define COMPILER_NAME __VERSION__
#elif defined(__GNUC__)
#define COMPILER_NAME "GCC " __VERSION__
#else
#define COMPILER_NAME "Unknown Compiler"
#endif

// Load the init program and execute it! This is the entry point of the userspace world.
[[noreturn]] static void load_init() {
  // Create the task manager.
  TaskManager& task_manager = TaskManager::get();
  auto init_task = task_manager.create_task("/bin/init");
  if (init_task == nullptr) {
    LOG_CRITICAL("Failed to load the init program");
  }

  // Execute the init program.
  LOG_INFO("Starting the init program");
  task_manager.wake_task(init_task);

  while (task_manager.get_current_task() != init_task)
    task_manager.schedule();

  // Enter userspace!
  enable_fpu_and_neon();
  init_task->get_saved_state().memory->activate();
  task_manager.mark_as_ready();
  jump_to_el0(init_task->get_saved_state().pc, (uintptr_t)init_task->get_saved_state().sp);
  LOG_CRITICAL("Exited from userspace");
}

[[noreturn]] void kmain() {
  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__ " with " COMPILER_NAME " !");

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  FileSystem::get().init();

  FrameBuffer& framebuffer = FrameBuffer::get();
  if (!framebuffer.init(1280, 720)) {
    LOG_WARNING("failed to initialize framebuffer");
  }

  WindowManager* window_manager = new WindowManager;
  KASSERT(window_manager != nullptr);

  // PS2Keyboard::init();
  // PS2Keyboard::set_on_event(&dispatch_key_event_to_wm);

  UART uart0(2000000, "uart0", /* irqs= */true);
  UARTKeyboard::init(&uart0);

  TaskManager* task_manager = new TaskManager;
  KASSERT(task_manager != nullptr);

  // Run the window manager task (thread).
  auto window_manager_task = task_manager->create_kernel_task([]() {
    while (true) {
      WindowManager::get().update();
      sys_usleep(63333);
    }
  });

  KASSERT(window_manager_task != nullptr);
  task_manager->wake_task(window_manager_task);

  // Run the init program.
  load_init();
}
