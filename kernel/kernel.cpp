#include "debug.hpp"
#include "dtb/dtb.hpp"
#include "graphics/graphics.hpp"
#include "graphics/pkfont.hpp"
#include "hardware/device.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/mmio.hpp"
#include "hardware/uart.hpp"

void print_property(const DeviceTree& dt, const char* property) {
  Property p;

  if (!dt.find_property(property, &p)) {
    LOG_CRITICAL("Unable to find property {} in device tree.", property);
  }

  LOG_INFO("DeviceTree property {} (size: {}) : {}", p.name, p.length, p.data);
}

extern "C" [[noreturn]] void kmain(const void* dtb) {
  const DeviceTree dt(dtb);
  MMIO::init(dt);
  UART::init(1000000);
  UART::puts("Hello, kernel World from UART!\r\n");


  LOG_INFO("DeviceTree initialization: {}", dt.is_status_okay());
  LOG_INFO("DeviceTree Version: {}", dt.get_version());
  print_property(dt, "/model");
  print_property(dt, "/compatible");
  print_property(dt, "/serial-number");

  Device device;
  LOG_INFO("Device initialization: {}", device.init());
  LOG_INFO("Board model: {}", device.get_board_model());
  LOG_INFO("Board revision: {}", device.get_board_revision());
  LOG_INFO("Board serial: {}", device.get_board_serial());
  LOG_INFO("ARM memory: {} bytes at {}", device.get_arm_memory_info().size, device.get_arm_memory_info().base_address);
  LOG_INFO("VC memory: {} bytes at {}", device.get_vc_memory_info().size, device.get_vc_memory_info().base_address);
  //  LOG_INFO("Temp: {} °C / {} °C", device.get_current_temp() / 1000.0f, device.get_max_temp() / 1000.0f);

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
