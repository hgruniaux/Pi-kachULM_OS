#include "hardware/mmio.hpp"
#include "hardware/uart.hpp"
#include "debug.hpp"
#include "device.hpp"
#include "framebuffer.hpp"
#include "graphics/graphics.hpp"
#include "graphics/pkfont.hpp"

// To move in a distinct file with the libk++
extern "C" void* memset(void* dest, int ch, size_t count) {
  auto* p = static_cast<unsigned char*>(dest);
  auto* p_end = static_cast<unsigned char*>(dest) + count;

  while (p != p_end) {
    *(p++) = static_cast<unsigned char>(ch);
  }

  return dest;
}

extern "C" [[noreturn]] void kmain() {
  MMIO::init();
  UART::init(115200);
  UART::puts("Hello, kernel World from UART!\r\n");

  Device device;
  LOG_INFO("Device initialization: {}", device.init());

  LOG_INFO("Board model: {}", device.get_board_model());
  LOG_INFO("Board revision: {}", device.get_board_revision());
  LOG_INFO("Board serial: {}", device.get_board_serial());
  LOG_INFO("ARM memory: {} bytes at {}", device.get_arm_memory_info().size, device.get_arm_memory_info().base_address);
  LOG_INFO("VC memory: {} bytes at {}", device.get_vc_memory_info().size, device.get_vc_memory_info().base_address);
  LOG_INFO("Temp: {} °C / {} °C", device.get_current_temp() / 1000.0f, device.get_max_temp() / 1000.0f);

  FrameBuffer& framebuffer = FrameBuffer::get();
  if (!framebuffer.init(640, 480)) {
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

  while (true) {
    UART::write_one(UART::read_one());
  }
}
