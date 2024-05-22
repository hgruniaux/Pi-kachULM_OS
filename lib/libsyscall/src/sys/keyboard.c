#include <sys/keyboard.h>

#define MASK(x) (1ull << x)
#define CTRL_MASK (MASK(20))
#define SHIFT_MASK (MASK(21))
#define ALT_MASK (MASK(22))
#define NUM_MASK (MASK(23))
#define CAP_MASK (MASK(24))
#define SCROLL_MASK (MASK(25))
#define KEY_CODE_MASK (0xffff)
#define PRESS_EVENT (MASK(30))
#define RELEASE_EVENT (MASK(31))

sys_bool_t sys_is_ctrl_pressed(sys_key_event_t event) {
  return (event & CTRL_MASK) != 0;
}

sys_bool_t sys_is_shift_pressed(sys_key_event_t event) {
  return (event & SHIFT_MASK) != 0;
}

sys_bool_t sys_is_alt_pressed(sys_key_event_t event) {
  return (event & ALT_MASK) != 0;
}

sys_bool_t sys_is_num_lock_on(sys_key_event_t event) {
  return (event & NUM_MASK) != 0;
}

sys_bool_t sys_is_caps_lock_on(sys_key_event_t event) {
  return (event & CAP_MASK) != 0;
}

sys_bool_t sys_is_scroll_lock_on(sys_key_event_t event) {
  return (event & SCROLL_MASK) != 0;
}

sys_key_code_t sys_get_key_code(sys_key_event_t event) {
  return (sys_key_code_t)(event & KEY_CODE_MASK);
}

sys_bool_t sys_is_press_event(sys_key_event_t event) {
  return (event & PRESS_EVENT) != 0;
}

sys_bool_t sys_is_release_event(sys_key_event_t event) {
  return (event & RELEASE_EVENT) != 0;
}

sys_key_event_t sys_create_press_event(sys_key_code_t code, sys_key_modifiers_t mods) {
  sys_key_event_t e = code;
  e |= (mods & SYS_KEY_MOD_CTRL) != 0 ? CTRL_MASK : 0;
  e |= (mods & SYS_KEY_MOD_SHIFT) != 0 ? SHIFT_MASK : 0;
  e |= (mods & SYS_KEY_MOD_ALT) != 0 ? ALT_MASK : 0;
  e |= (mods & SYS_KEY_MOD_NUM) != 0 ? NUM_MASK : 0;
  e |= (mods & SYS_KEY_MOD_CAPS) != 0 ? CAP_MASK : 0;
  e |= (mods & SYS_KEY_MOD_SCROLL) != 0 ? SCROLL_MASK : 0;
  e |= PRESS_EVENT;
  return e;
}

sys_key_event_t sys_create_release_event(sys_key_code_t code, sys_key_modifiers_t mods) {
  sys_key_event_t e = code;
  e |= (mods & SYS_KEY_MOD_CTRL) != 0 ? CTRL_MASK : 0;
  e |= (mods & SYS_KEY_MOD_SHIFT) != 0 ? SHIFT_MASK : 0;
  e |= (mods & SYS_KEY_MOD_ALT) != 0 ? ALT_MASK : 0;
  e |= (mods & SYS_KEY_MOD_NUM) != 0 ? NUM_MASK : 0;
  e |= (mods & SYS_KEY_MOD_CAPS) != 0 ? CAP_MASK : 0;
  e |= (mods & SYS_KEY_MOD_SCROLL) != 0 ? SCROLL_MASK : 0;
  e |= RELEASE_EVENT;
  return e;
}
