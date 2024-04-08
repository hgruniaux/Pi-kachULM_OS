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
}  // namespace libk
