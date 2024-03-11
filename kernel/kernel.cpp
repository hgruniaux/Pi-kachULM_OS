#include "debug.hpp"
#include "device.hpp"
#include "framebuffer.hpp"
#include "graphics/graphics.hpp"
#include "graphics/pkf.hpp"
#include "mmio.hpp"
#include "uart.hpp"

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count) {
  asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n" : "=r"(count) : [count] "0"(count) : "cc");
}

extern "C" void kmain() {
  MMIO::init();
  UART::init();

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

  auto line_color = graphics::make_color(255, 125, 0, 128);
  graphics::draw_line(50, 50, 200, 400, line_color);
  graphics::fill_rect(70, 80, 100, 200, line_color);

  auto text_color = graphics::make_color(20, 170, 200);
  graphics::draw_text("OS> kill", text_color, 50, 50);
  graphics::draw_text("OS> dump info", text_color, 50, 70);
  graphics::draw_text("OS> test", text_color, 50, 90);

  while (true) {
  }
}
