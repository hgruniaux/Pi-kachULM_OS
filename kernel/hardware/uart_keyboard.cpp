#include "uart_keyboard.hpp"
#include "hardware/irq/irq_lists.hpp"
#include "hardware/irq/irq_manager.hpp"
#include "input/keyboard_input.hpp"
#include "input/mouse_input.hpp"

namespace UARTKeyboard {
UART* keyboard_uart = nullptr;

void init(UART* uart) {
  KASSERT(uart != nullptr);
  keyboard_uart = uart;

  IRQManager::register_irq_handler(
      VC_UART,
      [](void*) {
        if (keyboard_uart->is_fifo_empty())
          return;

        uint8_t header = (uint8_t)keyboard_uart->read_one();

        switch (header & 0xF) {
          case 0x1:  // mouse move
          {
            uint8_t dx_mag = (uint8_t)keyboard_uart->read_one();
            uint8_t dy_mag = (uint8_t)keyboard_uart->read_one();

            int16_t dx = dx_mag;
            int16_t dy = dy_mag;

            if ((header & (1 << 4)) != 0)
              dx = -dx;
            if ((header & (1 << 5)) != 0)
              dy = -dy;

            MouseSystem::notify_hardware_move_event(dx, dy);
          } break;
          case 0x2:  // mouse click
          {
            uint8_t button = (uint8_t)keyboard_uart->read_one();
          } break;
          case 0x3:  // mouse scroll
          {
            uint8_t dx_mag = (uint8_t)keyboard_uart->read_one();
            uint8_t dy_mag = (uint8_t)keyboard_uart->read_one();

            int16_t dx = dx_mag;
            int16_t dy = dy_mag;

            if (header & (1 << 4) != 0)
              dx = -dx;
            if (header & (1 << 5) != 0)
              dy = -dy;

            MouseSystem::notify_hardware_scroll_event(dx, dy);
          } break;
          case 0x4:  // key press/release
          {
            uint16_t key;
            keyboard_uart->read((char*)&key, sizeof(key));

            bool is_pressed = (header & (1 << 4)) != 0;
            KeyboardSystem::notify_hardware_event((sys_key_code_t)key, is_pressed);
          } break;
        }
      },
      nullptr);

  LOG_INFO("UART keyboard & mouse driver initialized");
}
}  // namespace UARTKeyboard
