#pragma once

#include <cstddef>
#include <cstdint>

namespace MINI_UART {
/** Initializes Mini UART. */
void init(uint32_t baudrate);
/** Writes the given @a value into Mini UART. */
void write_one(uint8_t value);
/** Reads a single byte from UART1. */
uint8_t read_one();
/** Writes the given @a buffer of length @a buffer_length into Mini UART. */
void write(const uint8_t* buffer, size_t buffer_length);
/** Reads the requested @a buffer_length count of bytes into @a buffer from Mini UART. */
void read(uint8_t* buffer, size_t buffer_length);

/** Same as write() but takes a C NUL-terminated string.
 * Note that to print a new line, you must emit CR LF instead of only LF. */
void puts(const char* buffer);
};  // namespace UART
