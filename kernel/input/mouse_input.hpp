#pragma once

#include <cstdint>

namespace MouseSystem {
/** Max x and y coordinate for the mouse global position. */
static inline constexpr int32_t MAX_POSITION = INT16_MAX;
/** Min x and y coordinate for the mouse global position. */
static inline constexpr int32_t MIN_POSITION = INT16_MIN;

/** Gets the global x coordinate of the mouse.  */
int32_t get_mouse_x();
/** Gets the global y coordinate of the mouse.  */
int32_t get_mouse_y();

/** Moves the mouse to the given global coordinates. */
void move_mouse(int32_t x, int32_t y);

// These functions must be called from the hardware driver.
void notify_hardware_move_event(int32_t dx, int32_t dy);
void notify_hardware_scroll_event(int32_t dx, int32_t dy);
};  // namespace MouseSystem
