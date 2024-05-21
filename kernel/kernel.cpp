#include <libk/log.hpp>
#include "hardware/device.hpp"

#include "hardware/kernel_dt.hpp"

#include "fs/fat/ff.h"
#include "fs/filesystem.hpp"

#if defined(__GNUC__)
#define COMPILER_NAME "GCC " __VERSION__
#elif defined(__clang__)
#define COMPILER_NAME __VERSION__
#else
#define COMPILER_NAME "Unknown Compiler"
#endif

extern "C" const char init[];

[[noreturn]] void kmain() {
  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__ " with " COMPILER_NAME " !");

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  FileSystem::get().init();

  FIL f = {};
  LOG_DEBUG("Open '/test': {}", f_open(&f, "/test", FA_READ) == FR_OK);

  char buffer[1024] = {0};
  uint32_t byte_read;
  LOG_DEBUG("Reading '/test': {}", f_read(&f, buffer, 20, &byte_read) == FR_OK);

  LOG_DEBUG("Data read ({} bytes) : {:$}", byte_read, buffer);

  libk::halt();
}
