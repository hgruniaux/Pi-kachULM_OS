#pragma once

#include <source_location>

namespace libk {
[[noreturn]] void panic(const char* message, std::source_location source_location = std::source_location::current());
}  // namespace libk

#ifndef KASSERT
#define KASSERT(cond) (void)((cond) || (::libk::panic("assertion failed: " #cond, std::source_location::current()), 0))
#endif  // KASSERT

#ifndef KASSERT_X
#define KASSERT_X(cond, msg) (void)((cond) || (::libk::panic("assertion failed: " msg, std::source_location::current()), 0))
#endif // KASSERT_X

#ifdef assert
#undef assert
#endif // assert
#define assert(cond) KASSERT(cond)
