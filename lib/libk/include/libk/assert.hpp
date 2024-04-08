#pragma once

#include <source_location>

namespace libk {
[[noreturn]] void panic(const char* message, std::source_location source_location = std::source_location::current());
}  // namespace libk

#ifndef KASSERT
#define KASSERT(cond) (void)((cond) || (::libk::panic("assertion failed: " #cond, std::source_location::current()), 0))
#endif  // KASSERT
