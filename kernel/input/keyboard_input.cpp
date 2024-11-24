#include "keyboard_input.hpp"
#include "wm/window_manager.hpp"

namespace KeyboardSystem {
// We use integers instead of booleans to preserve the "is pressed" state
// to handle when pressing both the left and right variant of a button.
// For example:
//   - The user press LEFT CTRL (is_ctrl_pressed = 1)
//   - The user press RIGHT CTRL (is_ctrl_pressed = 2)
//   - The user releases LEFT CTRL (is_ctrl_pressed = 1 > 0 so CTRL is still pressed)
//   - The user releases RIGHT CTRL (is_ctrl_pressed = 0 si CTRL is not pressed anymore)
int is_ctrl_pressed = 0;
int is_shift_pressed = 0;
int is_alt_pressed = 0;
int is_alt_gr_pressed = 0;
int is_gui_pressed = 0;
bool is_nums_lock = false;
bool is_caps_lock = false;

sys_key_modifiers_t get_current_key_modifiers() {
  sys_key_modifiers_t mods = SYS_KEY_MOD_NONE;
  if (is_ctrl_pressed > 0)
    mods |= SYS_KEY_MOD_CTRL;
  if (is_shift_pressed > 0)
    mods |= SYS_KEY_MOD_SHIFT;
  if (is_alt_pressed > 0)
    mods |= SYS_KEY_MOD_ALT;
  if (is_nums_lock)
    mods |= SYS_KEY_MOD_NUM;
  if (is_caps_lock)
    mods |= SYS_KEY_MOD_CAPS;
  return mods;
}

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

void press(sys_key_code_t keycode) {
  switch (keycode) {
    case SYS_KEY_LEFT_CTRL:
    case SYS_KEY_RIGHT_CTRL:
      ++is_ctrl_pressed;
      break;
    case SYS_KEY_LEFT_SHIFT:
    case SYS_KEY_RIGHT_SHIFT:
      ++is_shift_pressed;
      break;
    case SYS_KEY_LEFT_ALT:
      ++is_alt_pressed;
      break;
    case SYS_KEY_RIGHT_ALT:
      ++is_alt_gr_pressed;
      break;
    case SYS_KEY_LEFT_GUI:
    case SYS_KEY_RIGHT_GUI:
      ++is_gui_pressed;
      break;
    case SYS_KEY_CAPS_LOCK:
      is_caps_lock = !is_caps_lock;
      break;
    case SYS_KEY_NUM_LOCK:
      is_nums_lock = !is_nums_lock;
      break;
    default:  // no special case
      break;
  }

  const auto modifiers = get_current_key_modifiers();
  const auto event = sys_create_press_event(keycode, modifiers);
  dispatch_key_event_to_wm(event);
}

void release(sys_key_code_t keycode) {
  switch (keycode) {
    case SYS_KEY_LEFT_CTRL:
    case SYS_KEY_RIGHT_CTRL:
      --is_ctrl_pressed;
      if (is_ctrl_pressed < 0)
        is_ctrl_pressed = 0;
      break;
    case SYS_KEY_LEFT_SHIFT:
    case SYS_KEY_RIGHT_SHIFT:
      --is_shift_pressed;
      if (is_shift_pressed < 0)
        is_shift_pressed = 0;
      break;
    case SYS_KEY_LEFT_ALT:
      --is_alt_pressed;
      if (is_alt_pressed < 0)
        is_alt_pressed = 0;
      break;
    case SYS_KEY_RIGHT_ALT:
      --is_alt_gr_pressed;
      if (is_alt_gr_pressed < 0)
        is_alt_gr_pressed = 0;
      break;
    case SYS_KEY_LEFT_GUI:
    case SYS_KEY_RIGHT_GUI:
      --is_gui_pressed;
      if (is_gui_pressed < 0)
        is_gui_pressed = 0;
      break;
    default:  // no special case
      break;
  }

  const auto modifiers = get_current_key_modifiers();
  const auto event = sys_create_release_event(keycode, modifiers);
  dispatch_key_event_to_wm(event);
}

void notify_hardware_event(sys_key_code_t keycode, bool is_pressed) {
  if (is_pressed)
    press(keycode);
  else
    release(keycode);
}
}  // namespace KeyboardSystem
