#include "mmio.hpp"
#include "uart.hpp"

// To move in a distinct file with the libk++
extern "C" void* memset(void* dest, int ch, size_t count) {
  auto* p = static_cast<unsigned char*>(dest);
  auto* p_end = static_cast<unsigned char*>(dest) + count;

  while (p != p_end) {
    *(p++) = static_cast<unsigned char>(ch);
  }

  return dest;
}

extern "C" [[noreturn]] void kmain() {
  MMIO::init();
  UART::init();
  UART::puts("Hello, kernel World!\r\n");

  while (true) {
    UART::write_one(UART::read_one());
  }
}
