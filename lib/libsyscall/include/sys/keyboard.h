#ifndef __PIKAOS_LIBC_SYS_KEYBOARD_H__
#define __PIKAOS_LIBC_SYS_KEYBOARD_H__

#include "__types.h"
#include "__utils.h"

__SYS_EXTERN_C_BEGIN

typedef enum sys_keycode_t {
  SYS_KEY_A = 0x1C,
  SYS_KEY_B = 0x32,
  SYS_KEY_C = 0x21,
  SYS_KEY_D = 0x23,
  SYS_KEY_E = 0x24,
  SYS_KEY_F = 0x2B,
  SYS_KEY_G = 0x34,
  SYS_KEY_H = 0x33,
  SYS_KEY_I = 0x43,
  SYS_KEY_J = 0x3B,
  SYS_KEY_K = 0x42,
  SYS_KEY_L = 0x4B,
  SYS_KEY_M = 0x3A,
  SYS_KEY_N = 0x31,
  SYS_KEY_O = 0x44,
  SYS_KEY_P = 0x4D,
  SYS_KEY_Q = 0x15,
  SYS_KEY_R = 0x2D,
  SYS_KEY_S = 0x1B,
  SYS_KEY_T = 0x2C,
  SYS_KEY_U = 0x3C,
  SYS_KEY_V = 0x2A,
  SYS_KEY_W = 0x1D,
  SYS_KEY_X = 0x22,
  SYS_KEY_Y = 0x35,
  SYS_KEY_Z = 0x1A,
  SYS_KEY_0 = 0x45,
  SYS_KEY_1 = 0x16,
  SYS_KEY_2 = 0x1E,
  SYS_KEY_3 = 0x26,
  SYS_KEY_4 = 0x25,
  SYS_KEY_5 = 0x2E,
  SYS_KEY_6 = 0x36,
  SYS_KEY_7 = 0x3D,
  SYS_KEY_8 = 0x3E,
  SYS_KEY_9 = 0x46,
  SYS_KEY_BACK_TICK = 0x0E,
  SYS_KEY_LESS_MORE = 0x61,
  SYS_KEY_MINUS = 0x4E,
  SYS_KEY_EQUAL = 0x55,
  SYS_KEY_BACKSLASH = 0x5D,
  SYS_KEY_BACKSPACE = 0x66,
  SYS_KEY_SPACE = 0x29,
  SYS_KEY_TAB = 0x0D,
  SYS_KEY_LEFT_GUI = 0xE01F,
  SYS_KEY_RIGHT_GUI = 0xE027,
  SYS_KEY_APPS = 0xE02F,
  SYS_KEY_ENTER = 0x5A,
  SYS_KEY_ESCAPE = 0x76,
  SYS_KEY_F1 = 0x05,
  SYS_KEY_F2 = 0x06,
  SYS_KEY_F3 = 0x04,
  SYS_KEY_F4 = 0x0C,
  SYS_KEY_F5 = 0x03,
  SYS_KEY_F6 = 0x0B,
  SYS_KEY_F7 = 0x83,
  SYS_KEY_F8 = 0x0A,
  SYS_KEY_F9 = 0x01,
  SYS_KEY_F10 = 0x09,
  SYS_KEY_F11 = 0x78,
  SYS_KEY_F12 = 0x07,
  SYS_KEY_PRINT_SCREEN = 0xE012,
  SYS_KEY_SCROLL = 0x7E,
  SYS_KEY_PAUSE = 0xE013,
  SYS_KEY_OPEN_BRACKET = 0x54,
  SYS_KEY_INSERT = 0xE070,
  SYS_KEY_HOME = 0xE06C,
  SYS_KEY_PAGE_UP = 0xE07D,
  SYS_KEY_DELETE = 0xE071,
  SYS_KEY_END = 0xE069,
  SYS_KEY_PAGE_DOWN = 0xE07A,
  SYS_KEY_UP_ARROW = 0xE075,
  SYS_KEY_LEFT_ARROW = 0xE06B,
  SYS_KEY_DOWN_ARROW = 0xE072,
  SYS_KEY_RIGHT_ARROW = 0xE074,
  SYS_KEY_NUMPAD_SLASH = 0xE04A,
  SYS_KEY_NUMPAD_STAR = 0x7C,
  SYS_KEY_NUMPAD_MINUS = 0x7B,
  SYS_KEY_NUMPAD_PLUS = 0x79,
  SYS_KEY_NUMPAD_ENTER = 0xE05A,
  SYS_KEY_NUMPAD_DOT = 0x71,
  SYS_KEY_NUMPAD_0 = 0x70,
  SYS_KEY_NUMPAD_1 = 0x69,
  SYS_KEY_NUMPAD_2 = 0x72,
  SYS_KEY_NUMPAD_3 = 0x7A,
  SYS_KEY_NUMPAD_4 = 0x6B,
  SYS_KEY_NUMPAD_5 = 0x73,
  SYS_KEY_NUMPAD_6 = 0x74,
  SYS_KEY_NUMPAD_7 = 0x6C,
  SYS_KEY_NUMPAD_8 = 0x75,
  SYS_KEY_NUMPAD_9 = 0x7D,
  SYS_KEY_CLOSE_BRACKET = 0x5B,
  SYS_KEY_SEMI_COLON = 0x4C,
  SYS_KEY_APOSTROPHE = 0x52,
  SYS_KEY_COMMA = 0x41,
  SYS_KEY_DOT = 0x49,
  SYS_KEY_SLASH = 0x4A,
} sys_key_code_t;

typedef enum sys_key_modifier_t {
  SYS_KEY_MOD_NONE = 0x0,
  SYS_KEY_MOD_CTRL = 0x1,
  SYS_KEY_MOD_SHIFT = 0x2,
  SYS_KEY_MOD_ALT = 0x4,
  SYS_KEY_MOD_NUM = 0x8,
  SYS_KEY_MOD_CAPS = 0x10,
  SYS_KEY_MOD_SCROLL = 0x20,
} sys_key_modifier_t;

typedef uint32_t sys_key_modifiers_t;

typedef uint64_t sys_key_event_t;

sys_bool_t sys_is_ctrl_pressed(sys_key_event_t event);
sys_bool_t sys_is_shift_pressed(sys_key_event_t event);
sys_bool_t sys_is_alt_pressed(sys_key_event_t event);

sys_bool_t sys_is_num_lock_on(sys_key_event_t event);
sys_bool_t sys_is_caps_lock_on(sys_key_event_t event);
sys_bool_t sys_is_scroll_lock_on(sys_key_event_t event);

sys_key_code_t sys_get_key_code(sys_key_event_t event);
sys_bool_t sys_is_press_event(sys_key_event_t event);
sys_bool_t sys_is_release_event(sys_key_event_t event);

sys_key_event_t sys_create_press_event(sys_key_code_t code, sys_key_modifiers_t mods);
sys_key_event_t sys_create_release_event(sys_key_code_t code, sys_key_modifiers_t mods);

__SYS_EXTERN_C_END

#endif  // __PIKAOS_LIBC_SYS_KEYBOARD_H__
