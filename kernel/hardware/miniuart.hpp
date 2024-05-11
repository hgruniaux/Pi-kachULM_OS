#pragma once

#include <cstddef>
#include <cstdint>
#include <libk/log.hpp>

class MiniUART : public libk::Logger {
 public:
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
  explicit MiniUART(uint32_t baud_rate);

  /** Writes the given @a value into Mini UART. */
  void write_one(char value) const;
  /** Reads a single byte from Mini UART. */
  char read_one() const;

  /** Writes the given @a buffer of length @a buffer_length into Mini UART. */
  void write(const char* buffer, size_t buffer_length) const;

  /** Writes the given @a buffer of length @a buffer_length into Mini UART.
   * This function add a newline after the buffer end */
  void writeln(const char* buffer, size_t buffer_length);

  /** Reads the requested @a buffer_length count of bytes into @a buffer from Mini UART. */
  void read(char* buffer, size_t buffer_length) const;

  /** Same as write() but takes a C NUL-terminated string.
   * Note that to print a new line, you must emit CR LF instead of only LF. */
  void puts(const char* buffer) const;

  bool support_colors() const { return true; }

 private:
  const uintptr_t _mini_uart_base;
};  // namespace MiniUART
