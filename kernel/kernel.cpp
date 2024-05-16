#include <libk/log.hpp>
#include "hardware/device.hpp"

#include "hardware/kernel_dt.hpp"
#include "hardware/system_timer.hpp"
#include "hardware/timer.hpp"
#include "hardware/uart.hpp"

#if defined(__GNUC__)
#define COMPILER_NAME "GCC " __VERSION__
#elif defined(__clang__)
#define COMPILER_NAME __VERSION__
#else
#define COMPILER_NAME "Unknown Compiler"
#endif

[[noreturn]] void kmain() {
  UART log(1000000);  // Set to a High Baud-rate, otherwise UART is THE bottleneck :/

  libk::register_logger(log);
  libk::set_log_timer(&GenericTimer::get_elapsed_time_in_ms);

  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__ " with " COMPILER_NAME " !");

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  LOG_INFO("Timer setup: {}", SystemTimer::set_recurrent_s(1, 1, []() { LOG_INFO("HELLLOOO !"); }));
  LOG_INFO("Timer setup: {}", SystemTimer::set_recurrent_ms(3, 333, []() { LOG_INFO("Whooo !"); }));

  while (true) {
    libk::wfi();
  }
}
