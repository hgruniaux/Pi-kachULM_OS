#pragma once

#include <source_location>

namespace debug {
[[noreturn]] void panic(const char* message, std::source_location source_location = std::source_location::current());
}  // namespace debug

#ifndef KASSERT
#define KASSERT(cond) (void)((cond) || (::debug::panic("assertion failed: " #cond), 0))
#endif  // KASSERT
