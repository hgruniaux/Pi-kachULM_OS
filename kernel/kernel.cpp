#include <libk/log.hpp>
#include "hardware/device.hpp"

#include "hardware/kernel_dt.hpp"
#include "hardware/system_timer.hpp"

#include "hardware/ps2_keyboard.hpp"
#include "hardware/uart.hpp"
#include "sys/keyboard.h"

#if defined(__GNUC__)
#define COMPILER_NAME "GCC " __VERSION__
#elif defined(__clang__)
#define COMPILER_NAME __VERSION__
#else
#define COMPILER_NAME "Unknown Compiler"
#endif

static key_event evs[100];
static size_t nb_evs = 0;

libk::StringView keycode_to_string(KeyCode key);

UART* uart = nullptr;

void print_event(key_event event) {
  if (is_press_event(event)) {
    LOG_INFO("Key Press: '{}' (Ctrl: {}, Alt: {}, Shift: {}, Num lock: {}, Caps lock: {}, Scroll lock: {})",
             keycode_to_string(get_key_code(event)), is_ctrl_pressed(event), is_alt_pressed(event),
             is_shift_pressed(event), is_num_lock_on(event), is_caps_lock_on(event), is_scroll_lock_on(event));
  }

  if (is_release_event(event)) {
    LOG_INFO("Key Release: '{}' (Ctrl: {}, Alt: {}, Shift: {}, Num lock: {}, Caps lock: {}, Scroll lock: {})",
             keycode_to_string(get_key_code(event)), is_ctrl_pressed(event), is_alt_pressed(event),
             is_shift_pressed(event), is_num_lock_on(event), is_caps_lock_on(event), is_scroll_lock_on(event));
  }
}

void on_tick() {
  for (size_t i = 0; i < nb_evs; ++i) {
    print_event(evs[i]);
  }

  nb_evs = 0;
}

void on_key_event(key_event ev) {
  if (is_release_event(ev)) {
    return;
  }

#define CASE_KEY_CHAR(k, chr) \
  case k: {                   \
    uart->write_one(chr);     \
    break;                    \
  }

  switch (get_key_code(ev)) {
    CASE_KEY_CHAR(KEY_A, 'a')
    CASE_KEY_CHAR(KEY_B, 'b')
    CASE_KEY_CHAR(KEY_C, 'c')
    CASE_KEY_CHAR(KEY_D, 'd')
    CASE_KEY_CHAR(KEY_E, 'e')
    CASE_KEY_CHAR(KEY_F, 'f')
    CASE_KEY_CHAR(KEY_G, 'g')
    CASE_KEY_CHAR(KEY_H, 'h')
    CASE_KEY_CHAR(KEY_I, 'i')
    CASE_KEY_CHAR(KEY_J, 'j')
    CASE_KEY_CHAR(KEY_K, 'k')
    CASE_KEY_CHAR(KEY_L, 'l')
    CASE_KEY_CHAR(KEY_M, 'm')
    CASE_KEY_CHAR(KEY_N, 'n')
    CASE_KEY_CHAR(KEY_O, 'o')
    CASE_KEY_CHAR(KEY_P, 'p')
    CASE_KEY_CHAR(KEY_Q, 'q')
    CASE_KEY_CHAR(KEY_R, 'r')
    CASE_KEY_CHAR(KEY_S, 's')
    CASE_KEY_CHAR(KEY_T, 't')
    CASE_KEY_CHAR(KEY_U, 'u')
    CASE_KEY_CHAR(KEY_V, 'v')
    CASE_KEY_CHAR(KEY_W, 'w')
    CASE_KEY_CHAR(KEY_X, 'x')
    CASE_KEY_CHAR(KEY_Y, 'y')
    CASE_KEY_CHAR(KEY_Z, 'z')
    CASE_KEY_CHAR(KEY_0, '0')
    CASE_KEY_CHAR(KEY_1, '1')
    CASE_KEY_CHAR(KEY_2, '2')
    CASE_KEY_CHAR(KEY_3, '3')
    CASE_KEY_CHAR(KEY_4, '4')
    CASE_KEY_CHAR(KEY_5, '5')
    CASE_KEY_CHAR(KEY_6, '6')
    CASE_KEY_CHAR(KEY_7, '7')
    CASE_KEY_CHAR(KEY_8, '8')
    CASE_KEY_CHAR(KEY_9, '9')
    CASE_KEY_CHAR(KEY_BACK_TICK, '`')
    CASE_KEY_CHAR(KEY_LESS_MORE, '<')
    CASE_KEY_CHAR(KEY_MINUS, '-')
    CASE_KEY_CHAR(KEY_EQUAL, '=')
    CASE_KEY_CHAR(KEY_BACKSLASH, '\\')
    CASE_KEY_CHAR(KEY_BACKSPACE, '\b')
    CASE_KEY_CHAR(KEY_SPACE, ' ')
    CASE_KEY_CHAR(KEY_TAB, '\t')
    CASE_KEY_CHAR(KEY_LEFT_GUI, 0x0)
    CASE_KEY_CHAR(KEY_RIGHT_GUI, 0x0)
    CASE_KEY_CHAR(KEY_APPS, 0x0)
    CASE_KEY_CHAR(KEY_ENTER, '\n')
    CASE_KEY_CHAR(KEY_ESCAPE, 0x1b)
    CASE_KEY_CHAR(KEY_F1, 0x0)
    CASE_KEY_CHAR(KEY_F2, 0x0)
    CASE_KEY_CHAR(KEY_F3, 0x0)
    CASE_KEY_CHAR(KEY_F4, 0x0)
    CASE_KEY_CHAR(KEY_F5, 0x0)
    CASE_KEY_CHAR(KEY_F6, 0x0)
    CASE_KEY_CHAR(KEY_F7, 0x0)
    CASE_KEY_CHAR(KEY_F8, 0x0)
    CASE_KEY_CHAR(KEY_F9, 0x0)
    CASE_KEY_CHAR(KEY_F10, 0x0)
    CASE_KEY_CHAR(KEY_F11, 0x0)
    CASE_KEY_CHAR(KEY_F12, 0x0)
    CASE_KEY_CHAR(KEY_PRINT_SCREEN, 0x0)
    CASE_KEY_CHAR(KEY_SCROLL, 0x0)
    CASE_KEY_CHAR(KEY_PAUSE, 0x0)
    CASE_KEY_CHAR(KEY_OPEN_BRACKET, '[')
    CASE_KEY_CHAR(KEY_INSERT, 0x0)
    CASE_KEY_CHAR(KEY_HOME, '\r')
    CASE_KEY_CHAR(KEY_PAGE_UP, 0x0)
    CASE_KEY_CHAR(KEY_DELETE, 0x0)
    CASE_KEY_CHAR(KEY_END, 0x3)
    CASE_KEY_CHAR(KEY_PAGE_DOWN, 0x0)
    CASE_KEY_CHAR(KEY_UP_ARROW, 0x0)
    CASE_KEY_CHAR(KEY_LEFT_ARROW, 0x0)
    CASE_KEY_CHAR(KEY_DOWN_ARROW, 0x0)
    CASE_KEY_CHAR(KEY_RIGHT_ARROW, 0x0)
    CASE_KEY_CHAR(KEY_NUMPAD_SLASH, '/')
    CASE_KEY_CHAR(KEY_NUMPAD_STAR, '*')
    CASE_KEY_CHAR(KEY_NUMPAD_MINUS, '-')
    CASE_KEY_CHAR(KEY_NUMPAD_PLUS, '+')
    CASE_KEY_CHAR(KEY_NUMPAD_ENTER, '\n')
    CASE_KEY_CHAR(KEY_NUMPAD_DOT, '.')
    CASE_KEY_CHAR(KEY_NUMPAD_0, '0')
    CASE_KEY_CHAR(KEY_NUMPAD_1, '1')
    CASE_KEY_CHAR(KEY_NUMPAD_2, '2')
    CASE_KEY_CHAR(KEY_NUMPAD_3, '3')
    CASE_KEY_CHAR(KEY_NUMPAD_4, '4')
    CASE_KEY_CHAR(KEY_NUMPAD_5, '5')
    CASE_KEY_CHAR(KEY_NUMPAD_6, '6')
    CASE_KEY_CHAR(KEY_NUMPAD_7, '7')
    CASE_KEY_CHAR(KEY_NUMPAD_8, '8')
    CASE_KEY_CHAR(KEY_NUMPAD_9, '9')
    CASE_KEY_CHAR(KEY_CLOSE_BRACKET, ']')
    CASE_KEY_CHAR(KEY_SEMI_COLON, ';')
    CASE_KEY_CHAR(KEY_APOSTROPHE, '\'')
    CASE_KEY_CHAR(KEY_COMMA, ',')
    CASE_KEY_CHAR(KEY_DOT, '.')
    CASE_KEY_CHAR(KEY_SLASH, '/')
  }
}

[[noreturn]] void kmain(UART& log) {
  uart = &log;
  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__ " with " COMPILER_NAME " !");

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  //  LOG_INFO("Timer: {}", SystemTimer::set_recurrent_s(1, 1, &on_tick));

  PS2Keyboard::init();
  PS2Keyboard::set_on_event(&on_key_event);
  //  PS2Keyboard::set_sticky_keys(true);

  while (true) {
    libk::wfi();
  }
}

libk::StringView keycode_to_string(KeyCode key) {
#define CASE_KEY(k) \
  case k: {         \
    return #k;      \
  }

  switch (key) {
    CASE_KEY(KEY_A)
    CASE_KEY(KEY_B)
    CASE_KEY(KEY_C)
    CASE_KEY(KEY_D)
    CASE_KEY(KEY_E)
    CASE_KEY(KEY_F)
    CASE_KEY(KEY_G)
    CASE_KEY(KEY_H)
    CASE_KEY(KEY_I)
    CASE_KEY(KEY_J)
    CASE_KEY(KEY_K)
    CASE_KEY(KEY_L)
    CASE_KEY(KEY_M)
    CASE_KEY(KEY_N)
    CASE_KEY(KEY_O)
    CASE_KEY(KEY_P)
    CASE_KEY(KEY_Q)
    CASE_KEY(KEY_R)
    CASE_KEY(KEY_S)
    CASE_KEY(KEY_T)
    CASE_KEY(KEY_U)
    CASE_KEY(KEY_V)
    CASE_KEY(KEY_W)
    CASE_KEY(KEY_X)
    CASE_KEY(KEY_Y)
    CASE_KEY(KEY_Z)
    CASE_KEY(KEY_0)
    CASE_KEY(KEY_1)
    CASE_KEY(KEY_2)
    CASE_KEY(KEY_3)
    CASE_KEY(KEY_4)
    CASE_KEY(KEY_5)
    CASE_KEY(KEY_6)
    CASE_KEY(KEY_7)
    CASE_KEY(KEY_8)
    CASE_KEY(KEY_9)
    CASE_KEY(KEY_BACK_TICK)
    CASE_KEY(KEY_LESS_MORE)
    CASE_KEY(KEY_MINUS)
    CASE_KEY(KEY_EQUAL)
    CASE_KEY(KEY_BACKSLASH)
    CASE_KEY(KEY_BACKSPACE)
    CASE_KEY(KEY_SPACE)
    CASE_KEY(KEY_TAB)
    CASE_KEY(KEY_LEFT_GUI)
    CASE_KEY(KEY_RIGHT_GUI)
    CASE_KEY(KEY_APPS)
    CASE_KEY(KEY_ENTER)
    CASE_KEY(KEY_ESCAPE)
    CASE_KEY(KEY_F1)
    CASE_KEY(KEY_F2)
    CASE_KEY(KEY_F3)
    CASE_KEY(KEY_F4)
    CASE_KEY(KEY_F5)
    CASE_KEY(KEY_F6)
    CASE_KEY(KEY_F7)
    CASE_KEY(KEY_F8)
    CASE_KEY(KEY_F9)
    CASE_KEY(KEY_F10)
    CASE_KEY(KEY_F11)
    CASE_KEY(KEY_F12)
    CASE_KEY(KEY_PRINT_SCREEN)
    CASE_KEY(KEY_SCROLL)
    CASE_KEY(KEY_PAUSE)
    CASE_KEY(KEY_OPEN_BRACKET)
    CASE_KEY(KEY_INSERT)
    CASE_KEY(KEY_HOME)
    CASE_KEY(KEY_PAGE_UP)
    CASE_KEY(KEY_DELETE)
    CASE_KEY(KEY_END)
    CASE_KEY(KEY_PAGE_DOWN)
    CASE_KEY(KEY_UP_ARROW)
    CASE_KEY(KEY_LEFT_ARROW)
    CASE_KEY(KEY_DOWN_ARROW)
    CASE_KEY(KEY_RIGHT_ARROW)
    CASE_KEY(KEY_NUMPAD_SLASH)
    CASE_KEY(KEY_NUMPAD_STAR)
    CASE_KEY(KEY_NUMPAD_MINUS)
    CASE_KEY(KEY_NUMPAD_PLUS)
    CASE_KEY(KEY_NUMPAD_ENTER)
    CASE_KEY(KEY_NUMPAD_DOT)
    CASE_KEY(KEY_NUMPAD_0)
    CASE_KEY(KEY_NUMPAD_1)
    CASE_KEY(KEY_NUMPAD_2)
    CASE_KEY(KEY_NUMPAD_3)
    CASE_KEY(KEY_NUMPAD_4)
    CASE_KEY(KEY_NUMPAD_5)
    CASE_KEY(KEY_NUMPAD_6)
    CASE_KEY(KEY_NUMPAD_7)
    CASE_KEY(KEY_NUMPAD_8)
    CASE_KEY(KEY_NUMPAD_9)
    CASE_KEY(KEY_CLOSE_BRACKET)
    CASE_KEY(KEY_SEMI_COLON)
    CASE_KEY(KEY_APOSTROPHE)
    CASE_KEY(KEY_COMMA)
    CASE_KEY(KEY_DOT)
    CASE_KEY(KEY_SLASH)
    default: {
      LOG_WARNING("Unknown Key Code: {:#x}", key);
      return "UNKNOWN";
    }
  }
}
