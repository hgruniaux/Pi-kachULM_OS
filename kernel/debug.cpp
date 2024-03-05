// Author: Hubert Gruniaux
// Date: 2/17/24
// The following code is released in the public domain (where applicable).

#include "debug.hpp"
#include "uart.hpp"

namespace debug {
#define CSI "\x1b["
#define COLOR(code) CSI code "m"
#define RESET COLOR("0")
#define BOLD COLOR("1")
#define ITALIC COLOR("3")
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

/// Converts the given unsigned 64-bit integer @a value to a string and store
/// the result into @a buffer. Returns the size of the output string.
///
/// The given @a buffer will be NUL-terminated, and in the worst case, must have
/// at least space for 21 characters (20 for the digits of the biggest integer
/// and 1 for the final NUL character).
///
/// If @a buffer is not big enough, behavior is undefined.
size_t uint_to_string(uint64_t value, char* buffer) {
  // TODO: Maybe move this function to elsewhere.

  size_t i = 0;

  // Handle the case when the number is 0 separately
  if (value == 0) {
    buffer[i++] = '0';
  }

  // Extract digits in reverse order
  while (value > 0) {
    const unsigned int digit = value % 10ull;
    buffer[i++] = digit + '0';
    value /= 10;
  }

  // Reverse the string to get the correct order
  size_t start = 0, end = i - 1;
  while (start < end) {
    char temp = buffer[start];
    buffer[start] = buffer[end];
    buffer[end] = temp;
    start++;
    end--;
  }

  // Null-terminate the string
  buffer[i] = '\0';
  return i;
}

/// Converts the given floating-point @a value to a string and store
/// the result into @a buffer. Returns the size of the output string.
///
/// The given @a buffer will be NUL-terminated, and in the worst case, must have
/// at least space for 42 characters (20 for the integer part, 20 for the
/// fractional part, 1 for the period and 1 for the final NUL character).
///
/// If @a buffer is not big enough, behavior is undefined.
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

static void print_header(Severity severity, const char* category, std::source_location source_location) {
  // Print source location if any.
  if (source_location.file_name() != nullptr && source_location.file_name()[0] != '\0') {
    UART::puts("[");
    UART::puts(source_location.file_name());

    // The C++ standard recommends to the set the line number to 0 if unknown.
    if (source_location.line() > 0) {
      UART::puts(":");
      // 21 bytes is enough for an unsigned 64-bit integer:
      //   - 20 characters for the digits (maximum digits in a 64-bit unsigned
      //   integer).
      //   - 1 character for the null terminator ('\0').
      char buffer[21] = {0};
      uint_to_string(source_location.line(), buffer);
      UART::puts(buffer);
    }

    // We can also print the function name. However, the GCC implementation
    // uses the __PRETTY_FUNCTION__ format which is a bit too verbose! So, for
    // now, we only print the file name and the line number.

    UART::puts("] ");
  }

  // Print category name if any.
  if (category != nullptr && category[0] != '\0') {
    UART::puts("[");
    UART::puts(category);
    UART::puts("] ");
  }

  // Print the message severity.
  const char* severity_header = kDEBUG_HEADER;
  switch (severity) {
    case Severity::TRACE:
      severity_header = kTRACE_HEADER;
      break;
    case Severity::DEBUG:
      severity_header = kDEBUG_HEADER;
      break;
    case Severity::INFO:
      severity_header = kINFO_HEADER;
      break;
    case Severity::WARNING:
      severity_header = kWARNING_HEADER;
      break;
    case Severity::ERROR:
      severity_header = kERROR_HEADER;
      break;
    case Severity::CRITICAL:
      severity_header = kCRITICAL_HEADER;
      break;
      // NOTE: avoid defining the default case. Let the compiler warn us when we
      // forget a severity level!
  }
  UART::puts(severity_header);
}

static void print_bool(bool value) {
  if (value) {
    UART::puts("true");
  } else {
    UART::puts("false");
  }
}

static void print_char(char value) {
  UART::write_one(value);
}

static void print_uint64(uint64_t value) {
  // 21 bytes is enough for an unsigned 64-bit integer:
  //   - 20 characters for the digits (maximum digits in a 64-bit unsigned
  //   integer).
  //   - 1 character for the null terminator ('\0').
  char buffer[21] = {0};
  uint_to_string(value, buffer);
  UART::puts(buffer);
}

static void print_int64(int64_t value) {
  if (value < 0) {
    UART::write_one('-');
    print_uint64(-value);
  } else {
    print_uint64(value);
  }
}

static void print_float(float value) {
  // TODO: maybe implement the ryu algorithm
  char buffer[42] = {0};
  float_to_string(value, buffer);
  UART::puts(buffer);
}

static void print_double(double value) {
  // TODO: maybe implement the ryu algorithm
  print_float((float)value);
}

static void print_c_string(const char* value) {
  UART::puts(value);
}

static void print_argument(const impl::Argument& argument) {
  switch (argument.type) {
    case impl::Argument::Type::BOOL:
      print_bool(argument.data.bool_value);
      return;
    case impl::Argument::Type::CHAR:
      print_char(argument.data.char_value);
      return;
    case impl::Argument::Type::INT64:
      print_int64(argument.data.int64_value);
      return;
    case impl::Argument::Type::UINT64:
      print_uint64(argument.data.uint64_value);
      return;
    case impl::Argument::Type::FLOAT:
      print_float(argument.data.float_value);
      return;
    case impl::Argument::Type::DOUBLE:
      print_double(argument.data.double_value);
      return;
    case impl::Argument::Type::C_STRING:
      print_c_string(argument.data.c_string_value);
      return;
    case impl::Argument::Type::POINTER:
      print_uint64(argument.data.uint64_value);
      return;
  }
  KASSERT(false && "unknown argument type");
}

static void print_message(std::source_location source_location,
                          const char* message,
                          const impl::Argument* args,
                          size_t args_count) {
  size_t current_arg_index = 0;
  const char* it = message;

  while (*it != '\0') {
    if (*it != '{') {
      UART::write_one(*it++);
      continue;
    }

    ++it;
    if (*it == '{') {  // '{{', escaped '{'
      ++it;
      UART::write_one('{');
    } else if (*it == '}') {  // An argument.
      ++it;

      if (current_arg_index >= args_count) {
        UART::puts("\r\n");  // print end line, so the panic message is on its own line
        // Oops, not enough arguments...
        panic("not enough arguments to debug message", source_location);
        return;
      }

      const impl::Argument& argument = args[current_arg_index];
      print_argument(argument);
      ++current_arg_index;
    } else {  // '{' followed by a dummy character... For now simply print the '{'
      UART::write_one(it[-1]);
    }
  }
}

void vlog(Severity severity,
          const char* category,
          const char* message,
          std::source_location source_location,
          const impl::Argument* args,
          size_t args_count) {
  // The log format we use:
  //   [file_name:line_number] [category] [severity] message

  print_header(severity, category, source_location);

  UART::puts(" ");
  print_message(source_location, message, args, args_count);
  // CR-LF is required here instead of just LF.
  // This is the way UART and QEMU terminal works.
  UART::puts("\r\n");
}

[[noreturn]] void panic(const char* message, std::source_location source_location) {
  log(Severity::CRITICAL, nullptr,
      RED BOLD "This is a kernel panic." RESET BOLD " DO NOT PANIC. Keep calm and carry on." RESET, source_location);
  log(Severity::CRITICAL, nullptr, message, source_location);

  // Enter an infinite loop, so we don't return from this function.
  while (true)
    ;
}
}  // namespace debug
