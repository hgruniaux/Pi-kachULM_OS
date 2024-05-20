#include <libk/log.hpp>
#include "hardware/device.hpp"

#include "graphics/graphics.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/interrupts.hpp"
#include "hardware/kernel_dt.hpp"

#include "sys/syscall.h"
#include "task/task_manager.hpp"
#include "wm/window_manager.hpp"

#if defined(__GNUC__)
#define COMPILER_NAME "GCC " __VERSION__
#elif defined(__clang__)
#define COMPILER_NAME __VERSION__
#else
#define COMPILER_NAME "Unknown Compiler"
#endif

extern "C" const char init[];

[[noreturn]] void kmain() {
  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__ " with " COMPILER_NAME " !");

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  FrameBuffer& framebuffer = FrameBuffer::get();
  if (!framebuffer.init(1280, 720)) {
    LOG_CRITICAL("failed to initialize framebuffer");
  }

  const uint32_t fb_width = framebuffer.get_width();
  const uint32_t fb_height = framebuffer.get_height();

  graphics::Painter painter;
  const char* text = "Hello kernel World from Graphics!";
  const PKFont font = painter.get_font();
  const uint32_t text_width = font.get_horizontal_advance(text);
  const uint32_t text_height = font.get_char_height();

  // Draw the text at the middle of screen
  painter.clear(graphics::Color::WHITE);
  painter.set_pen(graphics::Color::BLACK);
  painter.draw_text((fb_width - text_width) / 2, (fb_height - text_height) / 2, text);

  WindowManager* window_manager = new WindowManager;
  (void)window_manager;  // unused here, but we need to instance it.

  TaskManager* task_manager = new TaskManager;

  auto task1 = task_manager->create_task((const elf::Header*)&init);
  task_manager->wake_task(task1);

  auto task = task_manager->create_kernel_task([]() {
    while (true) {
      WindowManager::get().update();
      sys_usleep(63333);
    }
  });
  task_manager->wake_task(task);

  int count = 4;
  while (count-- > 0) {
    auto task = task_manager->create_task((const elf::Header*)&init);
    task_manager->wake_task(task);
  }

  enable_fpu_and_neon();
  //  Enter userspace
  task_manager->schedule();
  task_manager->get_current_task()->get_saved_state().memory->activate();
  jump_to_el0(task1->get_saved_state().pc, (uintptr_t)task_manager->get_current_task()->get_saved_state().sp);
  LOG_CRITICAL("Not in user space");
  libk::halt();
}
