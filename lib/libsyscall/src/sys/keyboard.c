#include "sys/keyboard.h"

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

bool is_ctrl_pressed(key_event event) {
  return (event & CTRL_MASK) != 0;
}

bool is_shift_pressed(key_event event) {
  return (event & SHIFT_MASK) != 0;
}

bool is_alt_pressed(key_event event) {
  return (event & ALT_MASK) != 0;
}

bool is_num_lock_on(key_event event) {
  return (event & NUM_MASK) != 0;
}

bool is_caps_lock_on(key_event event) {
  return (event & CAP_MASK) != 0;
}

bool is_scroll_lock_on(key_event event) {
  return (event & SCROLL_MASK) != 0;
}

KeyCode get_key_code(key_event event) {
  return (KeyCode)(event & KEY_CODE_MASK);
}

bool is_press_event(key_event event) {
  return (event & PRESS_EVENT) != 0;
}

bool is_release_event(key_event event) {
  return (event & RELEASE_EVENT) != 0;
}

key_event create_press_event(KeyCode code,
                             bool ctrl_on,
                             bool shift_on,
                             bool alt_on,
                             bool num_lock_on,
                             bool caps_on,
                             bool scroll_on) {
  key_event e = code;
  e |= ctrl_on ? CTRL_MASK : 0;
  e |= shift_on ? SHIFT_MASK : 0;
  e |= alt_on ? ALT_MASK : 0;
  e |= num_lock_on ? NUM_MASK : 0;
  e |= caps_on ? CAP_MASK : 0;
  e |= scroll_on ? SCROLL_MASK : 0;
  e |= PRESS_EVENT;

  return e;
}

key_event create_release_event(KeyCode code,
                               bool ctrl_on,
                               bool shift_on,
                               bool alt_on,
                               bool num_lock_on,
                               bool caps_on,
                               bool scroll_on) {
  key_event e = code;
  e |= ctrl_on ? CTRL_MASK : 0;
  e |= shift_on ? SHIFT_MASK : 0;
  e |= alt_on ? ALT_MASK : 0;
  e |= num_lock_on ? NUM_MASK : 0;
  e |= caps_on ? CAP_MASK : 0;
  e |= scroll_on ? SCROLL_MASK : 0;
  e |= RELEASE_EVENT;

  return e;
}
