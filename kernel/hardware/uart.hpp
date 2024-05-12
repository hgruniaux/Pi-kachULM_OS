#pragma once

#include <cstddef>
#include <cstdint>
#include <libk/log.hpp>

class UART : public libk::Logger {
 public:
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
  explicit UART(uint32_t baud_rate);

  /** Writes the given @a value into this UART. */
  void write_one(char value) const;
  /** Reads a single byte from this UART. */
  char read_one() const;

  /** Writes the given @a buffer of length @a buffer_length into this UART. */
  void write(const char* buffer, size_t buffer_length) const;

  /** Writes the given @a buffer of length @a buffer_length into this UART.
   * This function add a newline after the buffer end */
  void writeln(const char* buffer, size_t buffer_length);

  /** Reads the requested @a buffer_length count of bytes into @a buffer from this UART. */
  void read(char* buffer, size_t buffer_length) const;

  /** Same as write() but takes a C NUL-terminated string.
   * Note that to print a new line, you must emit CR LF instead of only LF. */
  void puts(const char* buffer) const;

  bool support_colors() const { return true; }

 private:
  const uintptr_t _uart_base;
};
