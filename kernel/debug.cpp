#include "debug.hpp"
#include "hardware/uart.hpp"

namespace debug {
// We don't use DEBUG_IMPL_LOGGER(default) because we wants a nullptr logger name. So it is not displayed by vlog().
Logger default_logger = {nullptr};

#define CSI "\x1b["
#define COLOR(code) CSI code "m"
#define RESET COLOR("0")
#define BOLD COLOR("1")
#define RED COLOR("31")
#define BACK_RED COLOR("41")
#define GREEN COLOR("32")
#define YELLOW COLOR("33")
#define BLUE COLOR("34")

constexpr const char* kTRACE_HEADER = "[trace]";
constexpr const char* kDEBUG_HEADER = "[" BLUE "debug" RESET "]";
constexpr const char* kINFO_HEADER = "[" GREEN "info" RESET "]";
constexpr const char* kWARNING_HEADER = "[" YELLOW "warning" RESET "]";
constexpr const char* kERROR_HEADER = "[" RED "error" RESET "]";
constexpr const char* kCRITICAL_HEADER = "[" BACK_RED "critical" RESET "]";

/** @brief Sinks logs and debug prints to UART0 serial port. */
class UARTSink {
 public:
  void putc(char ch) { UART::write_one((uint8_t)ch); }
  void puts(const char* buffer) { UART::puts(buffer); }
  void endline() {
    // CR-LF is required here instead of just LF.
    // This is the way UART works.
    UART::puts("\r\n");
  }
};  // class UARTSink

/** Converts the given unsigned 64-bit integer @a value to a string and store
 * the result into @a buffer. Returns the size of the output string.
 *
 * The given @a buffer will be NUL-terminated, and in the worst case, must have
 * at least space for 21 characters (20 for the digits of the biggest integer
 * and 1 for the final NUL character).
 *
 * If @a buffer is not big enough, behavior is undefined. */
size_t uint_to_string(uint64_t value, char* buffer) {
  // TODO: Maybe move this function to elsewhere.

  size_t i = 0;

  // Handle the case when the number is 0 separately
  if (value == 0) {
    buffer[i++] = '0';
  }

  // Extract digits in reverse order
  while (value > 0) {
    const char digit = (char)(value % 10ull);
    buffer[i++] = (char)(digit + '0');
    value /= 10;
  }

  // Reverse the string to get the correct order
  size_t start = 0, end = i - 1;
  while (start < end) {
    const char temp = buffer[start];
    buffer[start] = buffer[end];
    buffer[end] = temp;
    start++;
    end--;
  }

  // Null-terminate the string
  buffer[i] = '\0';
  return i;
}

/** Converts the given floating-point @a value to a string and store
 * the result into @a buffer. Returns the size of the output string.
 *
 * The given @a buffer will be NUL-terminated, and in the worst case, must have
 * at least space for 42 characters (20 for the integer part, 20 for the
 * fractional part, 1 for the period and 1 for the final NUL character).
 *
 * If @a buffer is not big enough, behavior is undefined. */
static size_t float_to_string(float value, char* buffer) {
  // TODO: Maybe move this function to elsewhere.
  // FIXME: this function only display 2 digits precision

  char* it = buffer;
  if (value < 0) {
    *it++ = '-';
    value *= -1;
  }

  // We add 0.005f to ensure correct rounding with 2 digits precision.
  // So 1.899 is printed as 1.90 instead of 1.89.
  value += 0.005f;

  // Print the integer part.
  const auto integer_part = (uint64_t)value;
  size_t written_bytes = uint_to_string(integer_part, it);
  it += written_bytes;

  *it++ = '.';
  ++written_bytes;

  // Print the fractional part. We only keep 2 digits after the period.
  value = (value - (float)integer_part) * 100 /* 2 digits */;
  written_bytes += uint_to_string((uint64_t)value, it);
  return written_bytes;
}

template <class Sink>
static void print_header(Sink& sink, Level level, const Logger& logger, std::source_location source_location) {
  // Print source location if any.
  if (source_location.file_name() != nullptr && source_location.file_name()[0] != '\0') {
    sink.puts("[");
    sink.puts(source_location.file_name());

    // The C++ standard recommends to the set the line number to 0 if unknown.
    if (source_location.line() > 0) {
      sink.puts(":");
      // 21 bytes is enough for an unsigned 64-bit integer:
      //   - 20 characters for the digits (maximum digits in a 64-bit unsigned
      //   integer).
      //   - 1 character for the null terminator ('\0').
      char buffer[21] = {0};
      uint_to_string(source_location.line(), buffer);
      sink.puts(buffer);
    }

    // We can also print the function name. However, the GCC implementation
    // uses the __PRETTY_FUNCTION__ format which is a bit too verbose! So, for
    // now, we only print the file name and the line number.

    sink.puts("] ");
  }

  // Print category name if any.
  if (logger.name != nullptr && logger.name[0] != '\0') {
    sink.puts("[");
    sink.puts(logger.name);
    sink.puts("] ");
  }

  // Print the message severity.
  const char* level_name = kDEBUG_HEADER;
  switch (level) {
    case Level::TRACE:
      level_name = kTRACE_HEADER;
      break;
    case Level::DEBUG:
      level_name = kDEBUG_HEADER;
      break;
    case Level::INFO:
      level_name = kINFO_HEADER;
      break;
    case Level::WARNING:
      level_name = kWARNING_HEADER;
      break;
    case Level::ERROR:
      level_name = kERROR_HEADER;
      break;
    case Level::CRITICAL:
      level_name = kCRITICAL_HEADER;
      break;
      // NOTE: avoid defining the default case. Let the compiler warn us when we
      // forget a severity level!
  }
  sink.puts(level_name);
}

template <class Sink>
static void print_bool(Sink& sink, bool value) {
  if (value) {
    sink.puts("true");
  } else {
    sink.puts("false");
  }
}

template <class Sink>
static void print_char(Sink& sink, char value) {
  sink.putc(value);
}

template <class Sink>
static void print_uint64(Sink& sink, uint64_t value) {
  // 21 bytes is enough for an unsigned 64-bit integer:
  //   - 20 characters for the digits (maximum digits in a 64-bit unsigned
  //   integer).
  //   - 1 character for the null terminator ('\0').
  char buffer[21] = {0};
  uint_to_string(value, buffer);
  sink.puts(buffer);
}

template <class Sink>
static void print_int64(Sink& sink, int64_t value) {
  if (value < 0) {
    sink.putc('-');
    print_uint64(sink, -value);
  } else {
    print_uint64(sink, value);
  }
}

template <class Sink>
static void print_float(Sink& sink, float value) {
  // TODO: maybe implement the ryu algorithm
  char buffer[42] = {0};
  float_to_string(value, buffer);
  sink.puts(buffer);
}

template <class Sink>
static void print_double(Sink& sink, double value) {
  // TODO: maybe implement the ryu algorithm
  print_float(sink, (float)value);
}

template <class Sink>
static void print_c_string(Sink& sink, const char* value) {
  sink.puts(value);
}

template <class Sink>
static void print_argument(Sink& sink, const impl::Argument& argument) {
  switch (argument.type) {
    case impl::Argument::Type::BOOL:
      print_bool(sink, argument.data.bool_value);
      return;
    case impl::Argument::Type::CHAR:
      print_char(sink, argument.data.char_value);
      return;
    case impl::Argument::Type::INT64:
      print_int64(sink, argument.data.int64_value);
      return;
    case impl::Argument::Type::UINT64:
      print_uint64(sink, argument.data.uint64_value);
      return;
    case impl::Argument::Type::FLOAT:
      print_float(sink, argument.data.float_value);
      return;
    case impl::Argument::Type::DOUBLE:
      print_double(sink, argument.data.double_value);
      return;
    case impl::Argument::Type::C_STRING:
      print_c_string(sink, argument.data.c_string_value);
      return;
    case impl::Argument::Type::POINTER:
      print_uint64(sink, argument.data.uint64_value);
      return;
  }
  KASSERT(false && "unknown argument type");
}

template <class Sink>
static void print_message(Sink& sink,
                          std::source_location source_location,
                          const char* message,
                          const impl::Argument* args,
                          size_t args_count) {
  size_t current_arg_index = 0;
  const char* it = message;

  while (*it != '\0') {
    if (*it != '{') {
      sink.putc(*it++);
      continue;
    }

    ++it;
    if (*it == '{') {  // '{{', escaped '{'
      ++it;
      sink.putc('{');
    } else if (*it == '}') {  // An argument.
      ++it;

      if (current_arg_index >= args_count) {
        // Oops, not enough arguments...
        sink.endline();  // print end line, so the panic message is on its own line
        panic("not enough arguments to debug message", source_location);
        return;
      }

      const impl::Argument& argument = args[current_arg_index];
      print_argument(sink, argument);
      ++current_arg_index;
    } else {  // '{' followed by a dummy character... For now simply print the '{'
      sink.putc(it[-1]);
    }
  }
}

void impl::vprint(const char* message, const impl::Argument* args, size_t args_count) {
  UARTSink uart_sink;
  print_message(uart_sink, std::source_location(), message, args, args_count);
  uart_sink.endline();
}

void impl::vlog(Level level,
                const Logger& logger,
                const char* message,
                std::source_location source_location,
                const impl::Argument* args,
                size_t args_count) {
  UARTSink uart_sink;

  // The log format we use:
  //   [file_name:line_number] [category] [level] message

  print_header(uart_sink, level, logger, source_location);

  uart_sink.puts(" ");
  print_message(uart_sink, source_location, message, args, args_count);
  uart_sink.endline();

  if (level == Level::CRITICAL) {
    panic("A critical log message was encountered.", source_location);
  }
}

[[noreturn]] void panic(const char* message, std::source_location source_location) {
  print(RED BOLD "KERNEL PANIC at {}:{} in `{}`." RESET, source_location.file_name(), source_location.line(),
        source_location.function_name());
  print(BOLD "Do not panic. Keep calm and carry on." RESET);
  print("Panic message: {}", message);

  // Enter an infinite loop, so we don't return from this function.
  while (true)
    ;
}
}  // namespace debug
