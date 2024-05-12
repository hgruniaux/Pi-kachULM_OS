#include <libk/log.hpp>
#include "hardware/device.hpp"
#include "hardware/uart.hpp"

#include "hardware/kernel_dt.hpp"
#include "hardware/timer.hpp"
#include "memory/memory.hpp"

[[noreturn]] void kmain() {
  UART log(1000000);  // Set to a High Baud-rate, otherwise UART is THE bottleneck :/

  libk::register_logger(log);
  libk::set_log_timer([]() { return GenericTimer::get_elapsed_time_in_ms(); });

  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__);

  ktest::run_tests();

#if 0
  LOG_INFO("DeviceTree initialization: {}", dt.is_status_okay());
  LOG_INFO("DeviceTree Version: {}", dt.get_version());
  print_property(dt, "/model");
  print_property(dt, "/compatible");
  print_property(dt, "/serial-number");
#endif
  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  LOG_INFO("Heap test start:");

  char* heap_start = (char*)KernelMemory::get_heap_end();
  LOG_INFO("Current kernel end: {:#x}", (uintptr_t)heap_start);

  constexpr const char* m_data = "Hello Kernel Heap!";
  constexpr size_t m_data_size = libk::strlen(m_data);
  LOG_INFO("New kernel end: {:#x}", KernelMemory::change_heap_end(m_data_size));

  libk::memcpy(heap_start, m_data, m_data_size);
  LOG_INFO("Written: {:$}", heap_start);
  LOG_INFO("Truc: {}", m_data_size);

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
