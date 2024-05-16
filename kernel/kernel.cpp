#include <libk/log.hpp>
#include "hardware/device.hpp"
#include "hardware/uart.hpp"

#include "hardware/kernel_dt.hpp"
#include "hardware/timer.hpp"
#include "libk/test.hpp"
#include "memory/memory.hpp"

#include "graphics/graphics.hpp"
#include "hardware/framebuffer.hpp"

[[noreturn]] void kmain() {
  UART log(1000000);  // Set to a High Baud-rate, otherwise UART is THE bottleneck :/

  libk::register_logger(log);
  libk::set_log_timer([]() { return GenericTimer::get_elapsed_time_in_ms(); });

  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__);

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  FrameBuffer& framebuffer = FrameBuffer::get();
  if (!framebuffer.init(1920, 1080)) {
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

  LOG_ERROR("END");
  while (true) {
    log.write_one(log.read_one());
  }
}
