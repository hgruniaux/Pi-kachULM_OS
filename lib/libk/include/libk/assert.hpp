#pragma once

#include <source_location>

namespace debug {
[[noreturn]] void panic(const char* message, std::source_location source_location);
}  // namespace debug

#ifndef KASSERT
#define KASSERT(cond) (void)((cond) || (::debug::panic("assertion failed: " #cond, std::source_location::current()), 0))
#endif  // KASSERT
