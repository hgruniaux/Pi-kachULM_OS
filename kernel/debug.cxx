// Author: Hubert Gruniaux
// Date: 2/17/24
// The following code is released in the public domain (where applicable).

#include "debug.hxx"
#include "uart.h"

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

void
int_to_string(uint64_t num, char* str)
{
  // TODO: Maybe move this function to elsewhere.

  int i = 0;

  // Handle the case when the number is 0 separately
  if (num == 0) {
    str[i++] = '0';
  }

  // Extract digits in reverse order
  while (num > 0) {
    int digit = num % 10;
    str[i++] = digit + '0';
    num /= 10;
  }

  // Reverse the string to get the correct order
  int start = 0, end = i - 1;
  while (start < end) {
    char temp = str[start];
    str[start] = str[end];
    str[end] = temp;
    start++;
    end--;
  }

  // Null-terminate the string
  str[i] = '\0';
}

void
log(Severity severity,
    const char* category,
    const char* message,
    std::source_location source_location)
{
  // The log format we use:
  //   [file_name:line_number] [category] [severity] message

  // Print source location if any.
  if (source_location.file_name() != nullptr &&
      source_location.file_name()[0] != '\0') {
    uart_puts("[");
    uart_puts(source_location.file_name());

    // The C++ standard recommends to the set the line number to 0 if unknown.
    if (source_location.line() > 0) {
      uart_puts(":");
      // 21 bytes is enough for an unsigned 64-bit integer:
      //   - 20 characters for the digits (maximum digits in a 64-bit unsigned
      //   integer).
      //   - 1 character for the null terminator ('\0').
      char buffer[21] = { 0 };
      int_to_string(source_location.line(), buffer);
      uart_puts(buffer);
    }

    // We can also print the function name. However, the GCC implementation
    // uses the __PRETTY_FUNCTION__ format which is a bit too verbose! So, for
    // now, we only print the file name and the line number.

    uart_puts("] ");
  }

  // Print category name if any.
  if (category != nullptr && category[0] != '\0') {
    uart_puts("[");
    uart_puts(category);
    uart_puts("] ");
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
  uart_puts(severity_header);

  uart_puts(" ");
  uart_puts(message);
  // CR-LF is required here instead of just LF.
  // This is the way UART and QEMU terminal works.
  uart_puts("\r\n");
}

void
panic(const char* message, std::source_location source_location)
{
  log(Severity::CRITICAL,
      nullptr,
      RED BOLD "This is a kernel panic." RESET BOLD
               " DO NOT PANIC. Keep calm and carry on." RESET,
      source_location);
  log(Severity::CRITICAL, nullptr, message, source_location);
}
} // namespace debug
