#include "mouse_input.hpp"
#include <libk/utils.hpp>
#include "wm/window_manager.hpp"

namespace MouseSystem {
// Absolute coordinates of the mouse current position.
int32_t mouse_x = 0, mouse_y = 0;

int32_t get_mouse_x() {
  return mouse_x;
}

int32_t get_mouse_y() {
  return mouse_y;
}

void move_mouse(int32_t x, int32_t y) {
  mouse_x = libk::clamp(x, MIN_POSITION, MAX_POSITION);
  mouse_y = libk::clamp(y, MIN_POSITION, MAX_POSITION);

  sys_message_t msg = {};
  msg.id = SYS_MSG_MOUSEMOVE;
  msg.param1 = mouse_x;
  msg.param2 = mouse_x;
  WindowManager::get().post_message(msg);
}

void notify_hardware_move_event(int32_t dx, int32_t dy) {
  move_mouse(mouse_x + dx, mouse_y + dy);
}

void notify_hardware_scroll_event(int32_t dx, int32_t dy) {
  // TODO
}
}  // namespace MouseSystem
