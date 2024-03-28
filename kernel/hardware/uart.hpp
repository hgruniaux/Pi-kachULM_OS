#pragma once

#include <cstddef>
#include <cstdint>

namespace UART {
/** Initializes the PL011 UART0 channel.
 * Accepted baud_rate :
 *  - 300     [Tested]
 *  - 600
 *  - 1200
 *  - 2400
 *  - 4800
 *  - 9600
 *  - 19200
 *  - 38400
 *  - 57600
 *  - 115200  [Tested]
 *  - 576000  [Tested]
 *  - 921600  [Tested]
 *  - 1000000 [Tested]
 *  - 1500000 [Tested]
 *  - 2000000 [Tested]
 */
void init(uint32_t baud_rate);
/** Writes the given @a value into UART0. */
void write_one(uint8_t value);
/** Reads a single byte from UART0. */
uint8_t read_one();
/** Writes the given @a buffer of length @a buffer_length into UART0. */
void write(const uint8_t* buffer, size_t buffer_length);
/** Reads the requested @a buffer_length count of bytes into @a buffer from UART0. */
void read(uint8_t* buffer, size_t buffer_length);

/** Same as write() but takes a C NUL-terminated string.
 * Note that to print a new line, you must emit CR LF instead of only LF. */
void puts(const char* buffer);
};  // namespace UART
