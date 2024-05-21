#ifndef __PIKAOS_LIBC_SYS_KEYBOARD_H__
#define __PIKAOS_LIBC_SYS_KEYBOARD_H__

#include "__utils.h"
#include "__types.h"

__SYS_EXTERN_C_BEGIN

typedef enum KeyCode {
  KEY_A = 0x1C,
  KEY_B = 0x32,
  KEY_C = 0x21,
  KEY_D = 0x23,
  KEY_E = 0x24,
  KEY_F = 0x2B,
  KEY_G = 0x34,
  KEY_H = 0x33,
  KEY_I = 0x43,
  KEY_J = 0x3B,
  KEY_K = 0x42,
  KEY_L = 0x4B,
  KEY_M = 0x3A,
  KEY_N = 0x31,
  KEY_O = 0x44,
  KEY_P = 0x4D,
  KEY_Q = 0x15,
  KEY_R = 0x2D,
  KEY_S = 0x1B,
  KEY_T = 0x2C,
  KEY_U = 0x3C,
  KEY_V = 0x2A,
  KEY_W = 0x1D,
  KEY_X = 0x22,
  KEY_Y = 0x35,
  KEY_Z = 0x1A,
  KEY_0 = 0x45,
  KEY_1 = 0x16,
  KEY_2 = 0x1E,
  KEY_3 = 0x26,
  KEY_4 = 0x25,
  KEY_5 = 0x2E,
  KEY_6 = 0x36,
  KEY_7 = 0x3D,
  KEY_8 = 0x3E,
  KEY_9 = 0x46,
  KEY_BACK_TICK = 0x0E,
  KEY_LESS_MORE = 0x61,
  KEY_MINUS = 0x4E,
  KEY_EQUAL = 0x55,
  KEY_BACKSLASH = 0x5D,
  KEY_BACKSPACE = 0x66,
  KEY_SPACE = 0x29,
  KEY_TAB = 0x0D,
  KEY_LEFT_GUI = 0xE01F,
  KEY_RIGHT_GUI = 0xE027,
  KEY_APPS = 0xE02F,
  KEY_ENTER = 0x5A,
  KEY_ESCAPE = 0x76,
  KEY_F1 = 0x05,
  KEY_F2 = 0x06,
  KEY_F3 = 0x04,
  KEY_F4 = 0x0C,
  KEY_F5 = 0x03,
  KEY_F6 = 0x0B,
  KEY_F7 = 0x83,
  KEY_F8 = 0x0A,
  KEY_F9 = 0x01,
  KEY_F10 = 0x09,
  KEY_F11 = 0x78,
  KEY_F12 = 0x07,
  KEY_PRINT_SCREEN = 0xE012,
  KEY_SCROLL = 0x7E,
  KEY_PAUSE = 0xE013,
  KEY_OPEN_BRACKET = 0x54,
  KEY_INSERT = 0xE070,
  KEY_HOME = 0xE06C,
  KEY_PAGE_UP = 0xE07D,
  KEY_DELETE = 0xE071,
  KEY_END = 0xE069,
  KEY_PAGE_DOWN = 0xE07A,
  KEY_UP_ARROW = 0xE075,
  KEY_LEFT_ARROW = 0xE06B,
  KEY_DOWN_ARROW = 0xE072,
  KEY_RIGHT_ARROW = 0xE074,
  KEY_NUMPAD_SLASH = 0xE04A,
  KEY_NUMPAD_STAR = 0x7C,
  KEY_NUMPAD_MINUS = 0x7B,
  KEY_NUMPAD_PLUS = 0x79,
  KEY_NUMPAD_ENTER = 0xE05A,
  KEY_NUMPAD_DOT = 0x71,
  KEY_NUMPAD_0 = 0x70,
  KEY_NUMPAD_1 = 0x69,
  KEY_NUMPAD_2 = 0x72,
  KEY_NUMPAD_3 = 0x7A,
  KEY_NUMPAD_4 = 0x6B,
  KEY_NUMPAD_5 = 0x73,
  KEY_NUMPAD_6 = 0x74,
  KEY_NUMPAD_7 = 0x6C,
  KEY_NUMPAD_8 = 0x75,
  KEY_NUMPAD_9 = 0x7D,
  KEY_CLOSE_BRACKET = 0x5B,
  KEY_SEMI_COLON = 0x4C,
  KEY_APOSTROPHE = 0x52,
  KEY_COMMA = 0x41,
  KEY_DOT = 0x49,
  KEY_SLASH = 0x4A,
} KeyCode;

typedef uint64_t key_event;

bool is_ctrl_pressed(key_event event);
bool is_shift_pressed(key_event event);
bool is_alt_pressed(key_event event);

bool is_num_lock_on(key_event event);
bool is_caps_lock_on(key_event event);
bool is_scroll_lock_on(key_event event);

KeyCode get_key_code(key_event event);
bool is_press_event(key_event event);
bool is_release_event(key_event event);

key_event
create_press_event(KeyCode code, bool ctrl_on, bool shift_on, bool alt_on, bool num_on, bool caps_on, bool scroll_on);
key_event
create_release_event(KeyCode code, bool ctrl_on, bool shift_on, bool alt_on, bool num_on, bool caps_on, bool scroll_on);

__SYS_EXTERN_C_END


#endif  // __PIKAOS_LIBC_SYS_KEYBOARD_H__
