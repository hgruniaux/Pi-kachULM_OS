#include <libk/log.hpp>
#include "hardware/device.hpp"
#include "hardware/uart.hpp"

#include "boot/mmu_utils.hpp"
#include "hardware/kernel_dt.hpp"
#include "hardware/timer.hpp"
#include "libk/test.hpp"
#include "memory/memory.hpp"

[[noreturn]] void kmain() {
  UART log(1000000);  // Set to a High Baud-rate, otherwise UART is THE bottleneck :/

  libk::register_logger(log);
  libk::set_log_timer([]() { return GenericTimer::get_elapsed_time_in_ms(); });

  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__);

  LOG_INFO("Conversion: {:#x}", KernelMemory::get_virtual_vc_address(0x3c100000));
  LOG_INFO("Conversion: {:#x}", _init_data.vc_offset);

  libk::write64(KernelMemory::get_virtual_vc_address(0x3c100000), 0xffff);

  //  SyscallManager::get().register_syscall(24, [](Registers& ) { LOG_INFO("Syscall 24"); });
  //
  //  //  Enter userspace
  //  jump_to_el0();
  //  asm volatile("mov w8, 24\n\tsvc #0");
  //  libk::halt();

  //  FrameBuffer& framebuffer = FrameBuffer::get();
  //  if (!framebuffer.init(640, 480)) {
  //    LOG_CRITICAL("failed to initialize framebuffer");
  //  }
  //
  //  const uint32_t fb_width = framebuffer.get_width();
  //  const uint32_t fb_height = framebuffer.get_height();
  //
  //  graphics::Painter painter;
  //
  //  const char* text = "Hello kernel World from Graphics!";
  //  const PKFont font = painter.get_font();
  //  const uint32_t text_width = font.get_horizontal_advance(text);
  //  const uint32_t text_height = font.get_char_height();
  //
  //   Draw the text at the middle of screen
  //  painter.clear(graphics::Color::WHITE);
  //  painter.set_pen(graphics::Color::BLACK);
  //  painter.draw_text((fb_width - text_width) / 2, (fb_height - text_height) / 2, text);
  //
  LOG_ERROR("END");
  while (true) {
    log.write_one(log.read_one());
  }
}
