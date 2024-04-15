#include "dtb/dtb.hpp"
#include "graphics/graphics.hpp"
#include "graphics/pkfont.hpp"
#include "hardware/device.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/interrupts.hpp"
#include "hardware/mmio.hpp"
#include "hardware/uart.hpp"
#include "mmu/mmu_kernel.hpp"
#include "syscall.hpp"

#include <libk/log.hpp>
#include "hardware/device.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/timer.hpp"
#include "memory/memory.hpp"
#include "memory/mmu_defs.hpp"

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

void find_and_dump(const DeviceTree dt, libk::StringView path) {
  Property p;

  if (!dt.find_property(path, &p)) {
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

class UARTLogger : public libk::Logger {
 public:
  bool support_colors() const override { return true; }
  void write(const char* data, size_t length) override {
    UART::write((const uint8_t*)data, length);
    UART::write((const uint8_t*)"\r\n", 2);
  }
};  // class UARTLogger

[[noreturn]] void kmain(const void* dtb) {
  const DeviceTree dt(dtb);
  MMIO::init();

  UART::init(115200);

  UARTLogger logger;
  libk::register_logger(logger);

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
  LOG_INFO("PGD0: {:#x}", libk::read64(resolve_symbol_pa(_mmu_init_data) + 0 * sizeof(uint64_t)));
  LOG_INFO("PGD1: {:#x}", libk::read64(resolve_symbol_pa(_mmu_init_data) + 1 * sizeof(uint64_t)));
  LOG_INFO("PGD2: {:#x}", libk::read64(resolve_symbol_pa(_mmu_init_data) + 2 * sizeof(uint64_t)));

  {
    uint64_t tmp;
    asm volatile("mov %x0, sp" : "=r"(tmp));
    LOG_INFO("SP : {:#x}", tmp);
  }

  const uintptr_t sp_start = KERNEL_BASE + STACK_SIZE;

  for (size_t i = sizeof(uint64_t); i < 0x100; i += sizeof(uint64_t)) {
    const uintptr_t cur_sp_ptr = sp_start - i;
    const uintptr_t shadow_sp_ptr = KERNEL_STACK_PAGE_BOTTOM((uint64_t)DEFAULT_CORE) + PAGE_SIZE - i;
    const auto cur_sp_val = libk::read64(cur_sp_ptr);
    const auto shadow_sp_val = libk::read64(shadow_sp_ptr);
    LOG_INFO("[Stack Dump] Index {}: Current: {:#x} (Address: {:#x}) Shadow: {:#x} (Address: {:#x})", i, cur_sp_val,
             cur_sp_ptr, shadow_sp_val, shadow_sp_ptr);
//    if (cur_sp_val != shadow_sp_val) {
//      LOG_CRITICAL("Aouch :/");
//    }
  }

  //  pp_stack(0xfff);
  //  fibo(100);
  // #if 0
  //  LOG_INFO("DeviceTree initialization: {}", dt.is_status_okay());
  //  LOG_INFO("DeviceTree Version: {}", dt.get_version());
  //  print_property(dt, "/model");
  //  print_property(dt, "/compatible");
  //  print_property(dt, "/serial-number");
  // #endif
  //
  //  Device device;
  //  LOG_INFO("Device initialization: {}", device.init());
  //  LOG_INFO("Board model: {}", device.get_board_model());
  //  LOG_INFO("Board revision: {}", device.get_board_revision());
  //  LOG_INFO("Board serial: {}", device.get_board_serial());
  //  LOG_INFO("ARM memory: {} bytes at {}", device.get_arm_memory_info().size,
  //  device.get_arm_memory_info().base_address); LOG_INFO("VC memory: {} bytes at {}",
  //  device.get_vc_memory_info().size, device.get_vc_memory_info().base_address); LOG_INFO("Temp: {} °C / {} °C",
  //  device.get_current_temp() / 1000, device.get_max_temp() / 1000);

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
  //  // Draw the text at the middle of screen
  //  painter.clear(graphics::Color::WHITE);
  //  painter.set_pen(graphics::Color::BLACK);
  //  painter.draw_text((fb_width - text_width) / 2, (fb_height - text_height) / 2, text);

  LOG_ERROR("END");
  while (true) {
    UART::write_one(UART::read_one());
  }
}
