#include "kernel.hpp"
#include "boot/device.hpp"
#include "hardware/interrupts.hpp"
#include "hardware/uart.hpp"

#include <libk/log.hpp>
#include "boot/kernel_dt.hpp"
#include "dtb/node.hpp"
#include "hardware/timer.hpp"

void print_property(const Property p) {
  if (p.is_string()) {
    LOG_INFO("DeviceTree property {} (size: {}) : '{}'", p.name, p.length, p.data);
  } else if (p.length % sizeof(uint32_t) == 0) {
    LOG_INFO("DeviceTree property {} (size: {}) :", p.name, p.length);
    const auto* data = (const uint32_t*)p.data;
    for (size_t i = 0; i < p.length / sizeof(uint32_t); i++) {
      LOG_INFO("  - At {}: {:#x}", i, libk::from_be(data[i]));
    }
  } else {
    LOG_INFO("DeviceTree property {} (size: {}) :", p.name, p.length);
    for (size_t i = 0; i < p.length; i++) {
      LOG_INFO("  - At {}: {:#x}", i, (uint8_t)p.data[i]);
    }
  }
}

void find_and_dump(libk::StringView path) {
  Property p;

  if (!KernelDT::find_property(path, &p)) {
    LOG_CRITICAL("Unable to find property: {}", path);
  }

  print_property(p);
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

[[noreturn]] void kmain() {
  UART u0(UART::Id::UART0, 115200);

  libk::register_logger(u0);

  libk::set_log_timer([]() { return GenericTimer::get_elapsed_time_in_ms(); });

  dump_current_el();
  init_interrupts_vector_table();

#define resolve_symbol_pa(symbol)                     \
  ({                                                  \
    uintptr_t __dest;                                 \
    asm volatile("adr %x0, " #symbol : "=r"(__dest)); \
    __dest;                                           \
  })

  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__);

#if 0
  LOG_INFO("DeviceTree initialization: {}", dt.is_status_okay());
  LOG_INFO("DeviceTree Version: {}", dt.get_version());
  print_property(dt, "/model");
  print_property(dt, "/compatible");
  print_property(dt, "/serial-number");
#endif

  // test_page_alloc();
  // test_bit_array();

  LOG_INFO("PGD0: {:#x}", libk::read64(resolve_symbol_pa(_mmu_init_data) + 0 * sizeof(uint64_t)));
  LOG_INFO("PGD1: {:#x}", libk::read64(resolve_symbol_pa(_mmu_init_data) + 1 * sizeof(uint64_t)));
  LOG_INFO("PGD2: {:#x}", libk::read64(resolve_symbol_pa(_mmu_init_data) + 2 * sizeof(uint64_t)));

  {
    uint64_t tmp;
    asm volatile("mov %x0, sp" : "=r"(tmp));
    LOG_INFO("SP : {:#x}", tmp);
  }

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

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
    u0.write_one(u0.read_one());
  }
}
