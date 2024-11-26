#pragma once

#include <sys/keyboard.h>

namespace KeyboardSystem {
void press(sys_key_code_t keycode);
void release(sys_key_code_t keycode);

// These functions must be called from the hardware driver.
void notify_hardware_event(sys_key_code_t keycode, bool is_pressed);
}  // namespace KeyboardSystem
