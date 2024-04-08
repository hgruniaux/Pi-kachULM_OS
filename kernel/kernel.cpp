#include "debug.hpp"
#include "dtb/dtb.hpp"
#include "graphics/graphics.hpp"
#include "graphics/pkfont.hpp"
#include "hardware/device.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/interrupts.hpp"
#include "hardware/mmio.hpp"
#include "hardware/uart.hpp"

void print_property(const DeviceTree& dt, const char* property) {
  Property p;

  if (!dt.find_property(property, &p)) {
    LOG_CRITICAL("Unable to find property {} in device tree.", property);
  }

  LOG_INFO("DeviceTree property {} (size: {}) : {}", p.name, p.length, p.data);
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

extern "C" [[noreturn]] void kmain(const void* dtb) {
  const DeviceTree dt(dtb);
#endif
  MMIO::init();
  UART::init(115200);

  LOG_INFO("Kernel built at " __DATE__ " " __TIME__);

#if 1
  dump_current_el();
  enable_fpu_and_neon();
  jump_to_el1();
  dump_current_el();
#endif

  init_interrupts_vector_table();

#if 0
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
