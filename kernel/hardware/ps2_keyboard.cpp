#include "ps2_keyboard.hpp"
#include <libk/log.hpp>
#include <libk/option.hpp>
#include "gpio.hpp"
#include "sys/keyboard.h"
#include "timer.hpp"

static inline constexpr size_t CLOCK_PIN = 23;
static inline constexpr size_t DATA_PIN = 24;

namespace PS2Keyboard {

static Event on_event = nullptr;
static bool is_sending = false;
static bool sticky = false;

libk::Option<uint8_t> receive_message() {
  static uint64_t message = 0;
  static uint64_t nb_bit = 0;
  static uint64_t parity = 0;

  static uint64_t last_update = 0;

  if (last_update == 0) {
    // Initialise Clock
    last_update = GenericTimer::get_elapsed_time_in_ms();
  }

  if (GenericTimer::get_elapsed_time_in_ms() - last_update > 10) {
    // Too long between 2 interrupts, we reset.
    message = 0;
    nb_bit = 0;
  }

  last_update = GenericTimer::get_elapsed_time_in_ms();

  nb_bit++;
  uint64_t bit_val = (uint64_t)GPIO::read(24);

  parity += bit_val;
  message |= (bit_val << 11);
  message >>= 1;

  if (nb_bit < 11) {
    return {};
  }

  nb_bit = 0;

  if (message & 0x1) {
    LOG_ERROR("[PS2Keyboard] Invalid start bit {:#x}", message);
    parity = 0;
    message = 0;
    return {};
  }

  if (!(message & 0x400)) {
    LOG_ERROR("[PS2Keyboard] Invalid stop bit {:#x}", message);
    parity = 0;
    message = 0;
    return {};
  }
  if ((((message & 0x200 >> 8) & 0x1) + (parity & 0x1)) & 0x1) {
    LOG_ERROR("[PS2Keyboard] Parity error {:#x} {:#x}", message, parity);
    parity = 0;
    message = 0;
    return {};
  }

  parity = 0;

  uint8_t key = (message >> 1) & 0xff;
  message = 0;
  return key;
}

void process_message(uint8_t message) {
  static bool ctrl_on = false;
  static bool shift_on = false;
  static bool alt_on = false;
  static bool num_on = false;
  static bool caps_on = false;
  static bool scroll_on = false;
  static bool is_release = false;
  static uint64_t msg_stack = 0;

  if (message == 0xf0) {
    is_release = true;
    return;
  }

  msg_stack = msg_stack << 8 | message;

  if (message >= 0x90) {
    // Not a Key code, wait for more...
    return;
  }

  if (msg_stack == 0xe012 || msg_stack == 0xe07c) {
    // Likely to be a Print Screen
    return;
  }

  if (msg_stack == 0xe114 || msg_stack == 0xe11477 || msg_stack == 0xe11477e114) {
    // Likely to be a Pause
    return;
  }

  if (msg_stack == 0x14 || msg_stack == 0xe014) {
    // Control Key press
    if (sticky) {
      if (is_release) {
        ctrl_on = !ctrl_on;
      }
    } else {
      if (is_release) {
        // We only update on Release
        ctrl_on = false;
      } else {
        ctrl_on = true;
      }
    }

    msg_stack = 0;
    is_release = false;
    return;
  }

  if (msg_stack == 0x12 || msg_stack == 0x59) {
    // Shift Key press
    if (sticky) {
      if (is_release) {
        // We only update on Release
        shift_on = !shift_on;
      }
    } else {
      if (is_release) {
        shift_on = false;
      } else {
        shift_on = true;
      }
    }

    msg_stack = 0;
    is_release = false;
    return;
  }

  if (msg_stack == 0x11 || msg_stack == 0xe011) {
    // Alt Key Press
    if (sticky) {
      if (is_release) {
        // We only update on Release
        alt_on = !alt_on;
      }
    } else {
      if (is_release) {
        alt_on = false;
      } else {
        alt_on = true;
      }
    }

    msg_stack = 0;
    is_release = false;
    return;
  }

  if (msg_stack == 0x77) {
    // Num Lock Screen Press

    if (is_release) {
      // We only update on Release
      num_on = !num_on;
    }

    msg_stack = 0;
    is_release = false;
    return;
  }

  if (msg_stack == 0x58) {
    // Caps Lock Screen Press

    if (is_release) {
      // We only update on Release
      caps_on = !caps_on;
    }

    msg_stack = 0;
    is_release = false;
    return;
  }

  if (msg_stack == 0x7e) {
    // Scroll Lock Screen Press

    if (is_release) {
      // We only update on Release
      scroll_on = !scroll_on;
    }

    msg_stack = 0;
    is_release = false;
    return;
  }

  if (msg_stack == 0xe012e07c || msg_stack == 0xe07ce012) {
    // Print Screen Press
    msg_stack = KEY_PRINT_SCREEN;
  }

  if (msg_stack == 0xe11477e11477) {
    // Pause Press
    msg_stack = KEY_PAUSE;
  }

  KeyCode keycode = (KeyCode)msg_stack;
  key_event event = is_release ? create_release_event(keycode, ctrl_on, shift_on, alt_on, num_on, caps_on, scroll_on)
                               : create_press_event(keycode, ctrl_on, shift_on, alt_on, num_on, caps_on, scroll_on);

  if (on_event != nullptr) {
    (*on_event)(event);
  }
  msg_stack = 0;
  is_release = false;
}

void gpio_clock_handler(size_t) {
  if (is_sending) {
  } else {
    auto msg = receive_message();
    if (msg.has_value()) {
      process_message(msg.get_value());
    }
  }
}

void init() {
  GPIO::set_mode(CLOCK_PIN, GPIO::Mode::INPUT);
  GPIO::set_mode(DATA_PIN, GPIO::Mode::INPUT);

  GPIO::set_event_callback(CLOCK_PIN, &gpio_clock_handler);

  GPIO::set_falling_edge_async_detect(CLOCK_PIN, true);
}

void set_on_event(Event ev) {
  on_event = ev;
}

void set_sticky_keys(bool enable) {
  sticky = enable;
}

}  // namespace PS2Keyboard
