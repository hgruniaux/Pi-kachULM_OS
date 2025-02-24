#include "mouse_input.hpp"
#include <libk/utils.hpp>
#include "wm/window_manager.hpp"

namespace MouseSystem {
void move_mouse(int32_t dx, int32_t dy) {
  sys_message_t msg = {};
  msg.id = SYS_MSG_MOUSEMOVE;
  msg.param1 = dx;
  msg.param2 = dy;
  WindowManager::get().post_message(msg);
}

void notify_hardware_move_event(int32_t dx, int32_t dy) {
  move_mouse(dx, dy);
}

void notify_hardware_scroll_event(int32_t dx, int32_t dy) {
  // TODO: implement mouse scroll
  (void)dx;
  (void)dy;
}

void notify_hardware_click_event(sys_mouse_button_t button, bool is_pressed) {
  sys_message_t msg = {};
  msg.id = SYS_MSG_MOUSECLICK;
  msg.param1 = button;
  msg.param2 = is_pressed;
  WindowManager::get().post_message(msg);
}
}  // namespace MouseSystem
