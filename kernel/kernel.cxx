#include "debug.hxx"
#include "uart.h"

extern "C" void
kmain(uint64_t dtb_ptr32, uint64_t x1, uint64_t x2, uint64_t x3)
{
  uart_init(3);
  uart_puts("Hello World\r\n");

  KASSERT(sizeof(int) > 8);

  while (1)
    uart_putc(uart_getc());
}
