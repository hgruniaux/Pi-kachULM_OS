// Author: Hubert Gruniaux
// Date: 2/17/24
// The following code is released in the public domain (where applicable).

#ifndef OS_UART_H
#define OS_UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void
uart_init(int raspi);

void
uart_putc(unsigned char c);

unsigned char
uart_getc();

void
uart_puts(const char* str);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // OS_UART_H
