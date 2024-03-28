#pragma once

#include <cstddef>
#include <cstdint>

namespace libk {
namespace detail {
struct Argument {
  /** @brief The different supported argument types. */
  enum class Type : uint8_t {
    /** @brief The `bool` type. */
    BOOL,
    /** @brief The `char` type. */
    CHAR,
    /** @brief The `intmax_t` type. */
    INTMAX,
    /** @brief The `uintmax_t` type. */
    UINTMAX,
    /** @brief The `float` type. */
    // FLOAT, // FIXME : Activate when floating point is activated
    /** @brief The `double` type. */
    // DOUBLE, // FIXME : Activate when floating point is activated
    /** @brief A pointer. */
    POINTER,
    /** @brief A NUL-terminated UTF-8 string. */
    C_STRING,
  };

  Type type;
  union {
    bool bool_value;
    char char_value;
    intmax_t intmax_value;
    uintmax_t uintmax_value;
    float float_value;
    double double_value;
    const char* c_string_value;
    const void* pointer_value;
  } data;

  Argument(bool value) : type(Type::BOOL) { data.bool_value = value; }
  Argument(char value) : type(Type::CHAR) { data.char_value = value; }
  Argument(int8_t value) : type(Type::INTMAX) { data.intmax_value = value; }
  Argument(int16_t value) : type(Type::INTMAX) { data.intmax_value = value; }
  Argument(int32_t value) : type(Type::INTMAX) { data.intmax_value = value; }
  Argument(int64_t value) : type(Type::INTMAX) { data.intmax_value = value; }
  Argument(uint8_t value) : type(Type::UINTMAX) { data.uintmax_value = value; }
  Argument(uint16_t value) : type(Type::UINTMAX) { data.uintmax_value = value; }
  Argument(uint32_t value) : type(Type::UINTMAX) { data.uintmax_value = value; }
  Argument(uint64_t value) : type(Type::UINTMAX) { data.uintmax_value = value; }
  // FIXME : Activate when floating point is activated
  // Argument(float value) : type(Type::FLOAT) { data.float_value = value; }
  // Argument(double value) : type(Type::DOUBLE) { data.double_value = value; }
  Argument(const char* value) : type(Type::C_STRING) { data.c_string_value = value; }
  Argument(const void* value) : type(Type::POINTER) { data.pointer_value = value; }
};  // struct Argument

char* format_to(char* out, const char* fmt, const Argument* args, size_t args_count);
}  // namespace detail

/**
 * Formats @a args according to specifications in @a fmt, writes the result to
 * the output iterator @a out and returns the iterator past the end of the
 * output range. format_to() does not append a terminating null character.
 *
 * The format string syntax is similar to the Python syntax and to the C++ fmt
 * library. A rigorous specification is provided at https://fmt.dev/latest/syntax.html.
 * However, not all features are supported but mostly all booleans, integers,
 * floating points, chars and strings formatting options are supported.
 *
 * Example:
 * ```c++
 * char buffer[1024] = {0};
 * char* it = libk::format_to(buffer, "The answer is {}. Note that {0} = {} + {1:#x}", 42, 2, 40);
 * *it = '\0';
 * // buffer will contain "The answer is 42. Note that 42 = 2 + 0x28".
 * ```
 */
template <typename... T>
char* format_to(char* out, const char* fmt, const T&... args) {
  detail::Argument args_array[] = {args...};
  return detail::format_to(out, fmt, args_array, sizeof...(T));
}
}  // namespace libk
