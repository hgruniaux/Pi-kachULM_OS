#pragma once

#include <cstdint>
#include <sys/window.h>

namespace MouseSystem {
/** Moves the mouse by the specified delta. */
void move_mouse(int32_t dx, int32_t dy);

// These functions must be called from the hardware driver.
void notify_hardware_move_event(int32_t dx, int32_t dy);
void notify_hardware_scroll_event(int32_t dx, int32_t dy);
void notify_hardware_click_event(sys_mouse_button_t button, bool is_pressed);
};  // namespace MouseSystem
