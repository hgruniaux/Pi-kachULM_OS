#pragma once

#include <libk/format.hpp>

namespace libk::detail {
/**
 * Formats a single argument to the given output @a out buffer following
 * the specified specification @a spec.
 */
char* format_argument_to(char* out, const Argument& argument, const char* spec);
}  // namespace libk::detail
