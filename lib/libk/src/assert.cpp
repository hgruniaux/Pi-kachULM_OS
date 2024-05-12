#include <libk/log.hpp>
#include <libk/utils.hpp>

namespace libk {
[[noreturn]] void panic(const char* message, std::source_location source_location) {
  print("KERNEL PANIC at {}:{} in `{}`.", source_location.file_name(), source_location.line(),
        source_location.function_name());
  print("Do not panic. Keep calm and carry on.");
  print("Panic message: {}", message);

  // Enter an infinite loop, so we don't return from this function.
  libk::halt();
}

// The following function are needed to use <cassert>

extern "C" [[noreturn]] void __assert_fail(const char* assertion,
                                           const char* file,
                                           unsigned int line,
                                           const char* function) noexcept(true) {
  print("In {}, at {}:{}: Assertion `{}' failed.", function, file, line, assertion);
  libk::halt();
}

extern "C" [[noreturn]] void __assert_perror_fail(int errnum,
                                                  const char* file,
                                                  unsigned int line,
                                                  const char* function) noexcept(true) {
  print("In {}, at {}:{}: Error number {}.", function, file, line, errnum);
  libk::halt();
};

extern "C" [[noreturn]] void __assert(const char* assertion, const char* file, int line) noexcept(true) {
  print("At {}:{}: Assertion `{}' failed.", file, line, assertion);
  libk::halt();
}

}  // namespace libk
