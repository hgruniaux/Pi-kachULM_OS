#include <libk/log.hpp>
#include "hardware/device.hpp"

#include "graphics/graphics.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/interrupts.hpp"
#include "hardware/kernel_dt.hpp"
#include "hardware/system_timer.hpp"
#include "hardware/timer.hpp"
#include "hardware/uart.hpp"

#include "task/task_manager.hpp"

#if defined(__GNUC__)
#define COMPILER_NAME "GCC " __VERSION__
#elif defined(__clang__)
#define COMPILER_NAME __VERSION__
#else
#define COMPILER_NAME "Unknown Compiler"
#endif

static inline constexpr uint32_t CHRONO_MS_PERIOD = 0x100;

uint16_t ms = 0;
uint8_t s = 0;
uint64_t m = 0;

template <class T>
void int_to_hex_string(char* buffer, T value) {
  static const char hex_digits[] = "0123456789ABCDEF";

  for (size_t i = 0; i < sizeof(T) * 2; ++i) {
    buffer[i] = hex_digits[(value >> (4 * (sizeof(T) * 2 - (i + 1)))) & 0xf];
  }

  buffer[sizeof(T) * 2] = 0;
}

void update_timer() {
  ms += CHRONO_MS_PERIOD;
  if (ms >= 1000) {
    s += ms / 1000;
    ms %= 1000;

    if (s >= 60) {
      m += s / 60;
      s %= 60;
    }
  }
}

void draw_timer() {
  static char buffer[sizeof(uint64_t) * 4 + 1] = {0};
  graphics::Painter painter;

  painter.set_pen(graphics::Color::WHITE);
  painter.fill_rect(50, 50, 400, 100);
  painter.set_pen(graphics::Color::RED);
  painter.draw_rect(50, 50, 400, 100);

  // Draw the text at the middle of screen
  painter.set_pen(graphics::Color::BLACK);

  painter.draw_text(50, 50, "Hex Clock: ");

  int_to_hex_string(buffer, m);
  painter.draw_text(50, 70, buffer);

  int_to_hex_string(buffer, s);
  painter.draw_text(250, 70, buffer);

  int_to_hex_string(buffer, ms);
  painter.draw_text(300, 70, buffer);
}

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

  TaskManager* task_manager = new TaskManager;

  auto task1 = task_manager->create_task((const elf::Header*)&init);
  task_manager->wake_task(task1);

  auto task2 = task_manager->create_task((const elf::Header*)&init);
  task_manager->wake_task(task2);

  auto task3 = task_manager->create_task((const elf::Header*)&init);
  task_manager->wake_task(task3);

  //  Enter userspace
  task_manager->schedule();
  task_manager->get_current_task()->get_saved_state().memory->activate();
  jump_to_el0(task1->get_saved_state().regs.elr, (uintptr_t)task_manager->get_current_task()->get_saved_state().sp);
  LOG_CRITICAL("Not in user space");

  while (true) {
    libk::wfi();
  }
}
