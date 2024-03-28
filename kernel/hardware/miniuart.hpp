#pragma once

#include <cstddef>
#include <cstdint>

namespace MiniUART {
/** Initializes the Mini UART.
 * Accepted baud_rate :
 *  - 9600    [Tested]
 *  - 19200   [Tested]
 *  - 38400
 *  - 57600
 *  - 115200  [Tested]
 *  - 576000
 *  - 921600
 *  - 1000000 [Tested]
 *  - 1500000 [Tested]
 *  - 2000000 *UNSTABLE*
 */
void init(uint32_t baud_rate);
/** Writes the given @a value into Mini UART. */
void write_one(uint8_t value);
/** Reads a single byte from Mini UART. */
uint8_t read_one();
/** Writes the given @a buffer of length @a buffer_length into Mini UART. */
void write(const uint8_t* buffer, size_t buffer_length);
/** Reads the requested @a buffer_length count of bytes into @a buffer from Mini UART. */
void read(uint8_t* buffer, size_t buffer_length);

/** Same as write() but takes a C NUL-terminated string.
 * Note that to print a new line, you must emit CR LF instead of only LF. */
void puts(const char* buffer);
};  // namespace MiniUART
