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
#include "hardware/timer.hpp"

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

void test_bit_array() {
  uint64_t mon_tableau[2];
  libk::BitArray test = libk::BitArray(mon_tableau,sizeof(mon_tableau)*8);
  // test.set_bit((size_t)0, true);
  // libk::print("Le premier bit est {}", test.get_bit((size_t)0));
  // libk::print("Le second bit est {}", test.get_bit((size_t)1));
  test.set_bit((size_t) 3, false);
  for (size_t i = 0; i <= 65; i++)
  {
    libk::print("Le bit {} est {}", i, test.get_bit(i));
  }
}

void test_page_alloc() {  // ATTENTION nb_page = multiple de 64
  // test du PageAlloc
  libk::print("Test begin");
  PageAlloc test = PageAlloc((uint64_t)(64 * PAGESIZE));
  uint64_t tab_support[2];
  test.setmmap(tab_support);
  test.mark_as_used((physical_address_t) (31*PAGESIZE));
  libk::print("Mark 1");
  physical_address_t addr;
  test.freshpage(&addr);
  libk::print("Mark 2");
  libk::print("Le statut de la page {} est {}", addr, test.page_status(addr));
  while (test.freshpage(&addr)) {
  };
  test.freepage((physical_address_t)0);
  libk::print("Mark 3");
  libk::print("Le statut de la première page est {}", test.page_status((physical_address_t)0));
  if (test.freshpage(&addr)) {
    libk::print("Test succeded !");
  } else {
    libk::print("Test failed ...");
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

extern "C" [[noreturn]] void kmain(const void* dtb) {
#if 0
  const DeviceTree dt(dtb);
#endif
  MMIO::init();
  UART::init(115200);

  UARTLogger logger;
  libk::register_logger(logger);
  libk::set_log_timer([]() { return GenericTimer::get_elapsed_time_in_ms(); });

  dump_current_el();
  enable_fpu_and_neon();
  jump_to_el1();
  dump_current_el();

  init_interrupts_vector_table();

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

  Device device;
  LOG_INFO("Device initialization: {}", device.init());
  LOG_INFO("Board model: {}", device.get_board_model());
  LOG_INFO("Board revision: {}", device.get_board_revision());
  LOG_INFO("Board serial: {}", device.get_board_serial());
  LOG_INFO("ARM memory: {} bytes at {}", device.get_arm_memory_info().size, device.get_arm_memory_info().base_address);
  LOG_INFO("VC memory: {} bytes at {}", device.get_vc_memory_info().size, device.get_vc_memory_info().base_address);
  // LOG_INFO("Temp: {} °C / {} °C", device.get_current_temp() / 1000.0f, device.get_max_temp() / 1000.0f);

  SyscallManager::get().register_syscall(24, [](Registers& regs) { LOG_INFO("Syscall 24"); });

  // Enter userspace
  jump_to_el0();
  asm volatile("mov w8, 24\n\tsvc #0");
  libk::halt();

#if 0
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
#endif

  while (true) {
    UART::write_one(UART::read_one());
  }
}
