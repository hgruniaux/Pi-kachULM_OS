#pragma once

#include <cstdint>
#include "hardware/uart.hpp"

namespace UARTKeyboard {
void init(UART* uart);
}  // namespace UARTKeyboard
