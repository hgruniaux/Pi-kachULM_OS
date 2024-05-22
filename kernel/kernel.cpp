#include <libk/log.hpp>
#include "hardware/device.hpp"

#include "hardware/kernel_dt.hpp"
#include "hardware/system_timer.hpp"

#include "hardware/ps2_keyboard.hpp"
#include "hardware/uart.hpp"
#include "sys/keyboard.h"
#include "sys/window.h"
#include "wm/window_manager.hpp"

#if defined(__GNUC__)
#define COMPILER_NAME "GCC " __VERSION__
#elif defined(__clang__)
#define COMPILER_NAME __VERSION__
#else
#define COMPILER_NAME "Unknown Compiler"
#endif

// Send a key event to the window manager.
static void dispatch_key_event_to_wm(sys_key_event_t event) {
  sys_message_t msg = {};
  if (sys_is_press_event(event))
    msg.id = SYS_MSG_KEYDOWN;
  else if (sys_is_release_event(event))
    msg.id = SYS_MSG_KEYUP;
  else
    return;

  msg.param1 = event;
  msg.param2 = 0;
  WindowManager::get().post_message(msg);
}

[[noreturn]] void kmain() {
  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__ " with " COMPILER_NAME " !");

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  PS2Keyboard::init();
  PS2Keyboard::set_on_event(&dispatch_key_event_to_wm);

  while (true) {
    libk::wfi();
  }
}
