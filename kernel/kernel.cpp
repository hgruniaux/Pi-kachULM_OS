#include <cstddef>
#include <cstdint>

#include "mmio.hpp"
#include "uart.hpp"

extern "C" void kmain() {
  MMIO::init();
  UART::init();
  UART::puts("Hello, kernel World!\r\n");

  while (true)
    UART::write_one(UART::read_one());
}
