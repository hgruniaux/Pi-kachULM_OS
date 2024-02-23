// Author: Hubert Gruniaux
// Date: 2/17/24
// The following code is released in the public domain (where applicable).

#ifndef OS_DEBUG_HXX
#define OS_DEBUG_HXX

#include <cstddef>
#include <cstdint>
#include <source_location>

namespace debug {
namespace impl {
struct Argument
{
  enum class Type
  {
    /// The `bool` type.
    BOOL,
    /// The `char` type.
    CHAR,
    /// The `int64_t` type.
    INT64,
    /// The `uint64_t` type.
    UINT64,
    /// The `float` type.
    FLOAT,
    /// The `double` type.
    DOUBLE,
    /// A pointer.
    POINTER,
    /// A NUL-terminated UTF-8 string.
    C_STRING,
  };

  Type type;
  union
  {
    bool bool_value;
    char char_value;
    int64_t int64_value;
    uint64_t uint64_value;
    float float_value;
    double double_value;
    const char* c_string_value;
    const void* pointer_value;
  } data;

  // clang-format off
  Argument(bool value) : type(Type::BOOL) { data.bool_value = value; }
  Argument(char value) : type(Type::CHAR) { data.char_value = value; }
  Argument(int8_t value) : type(Type::INT64) { data.int64_value = value; }
  Argument(int16_t value) : type(Type::INT64) { data.int64_value = value; }
  Argument(int32_t value) : type(Type::INT64) { data.int64_value = value; }
  Argument(int64_t value) : type(Type::INT64) { data.int64_value = value; }
  Argument(uint8_t value) : type(Type::UINT64) { data.uint64_value = value; }
  Argument(uint16_t value) : type(Type::UINT64) { data.uint64_value = value; }
  Argument(uint32_t value) : type(Type::UINT64) { data.uint64_value = value; }
  Argument(uint64_t value) : type(Type::UINT64) { data.uint64_value = value; }
  Argument(float value) : type(Type::FLOAT) { data.float_value = value; }
  Argument(double value) : type(Type::DOUBLE) { data.double_value = value; }
  Argument(const char* value) : type(Type::C_STRING) { data.c_string_value = value; }
  Argument(const void* value) : type(Type::POINTER) { data.pointer_value = value; }
  // clang-format on
}; // class Argument
} // namespace impl

/// The different severity levels supported by the debug module.
///
/// The levels have no specific semantic. However, a different formatting is
/// used for each of them and it is possible to block messages with a low
/// severity.
enum class Severity
{
  TRACE,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  CRITICAL
}; // enum class Severity

void
vlog(Severity severity,
     const char* category,
     const char* message,
     std::source_location source_location,
     const impl::Argument* args,
     size_t args_count);

template<class... Args>
inline void
log(Severity severity,
    const char* category,
    const char* message,
    std::source_location source_location,
    const Args&... args)
{
  impl::Argument args_array[] = { args... };
  vlog(severity, category, message, source_location, args_array, sizeof...(Args));
}

template<class... Args>
inline void
trace(const char* category, const char* message, std::source_location source_location, const Args&... args)
{
  log(Severity::TRACE, category, message, source_location, args...);
}

template<class... Args>
inline void
debug(const char* category, const char* message, std::source_location source_location, const Args&... args)
{
  log(Severity::DEBUG, category, message, source_location, args...);
}

template<class... Args>
inline void
info(const char* category, const char* message, std::source_location source_location, const Args&... args)
{
  log(Severity::INFO, category, message, source_location, args...);
}

template<class... Args>
inline void
warning(const char* category, const char* message, std::source_location source_location, const Args&... args)
{
  log(Severity::WARNING, category, message, source_location, args...);
}

template<class... Args>
inline void
error(const char* category, const char* message, std::source_location source_location, const Args&... args)
{
  log(Severity::ERROR, category, message, source_location, args...);
}

template<class... Args>
inline void
critical(const char* category, const char* message, std::source_location source_location, const Args&... args)
{
  log(Severity::CRITICAL, category, message, source_location, args...);
}

[[noreturn]] void
panic(const char* message, std::source_location source_location = std::source_location::current());
} // namespace debug

/// See debug::Severity::Trace.
#define LOG_TRACE_LEVEL 0
/// See debug::Severity::Debug.
#define LOG_DEBUG_LEVEL 1
/// See debug::Severity::Info.
#define LOG_INFO_LEVEL 2
/// See debug::Severity::Warning.
#define LOG_WARNING_LEVEL 3
/// See debug::Severity::Error.
#define LOG_ERROR_LEVEL 4
/// See debug::Severity::Critical.
#define LOG_CRITICAL_LEVEL 5

#ifndef LOG_MIN_LEVEL
/// LOG_MIN_LEVEL defines the minimum severity level to be logged.
/// It can be defined to one of:
///   - LOG_DEBUG_LEVEL: show all messages including debug ones
///   - LOG_INFO_LEVEL
///   - LOG_WARNING_LEVEL
///   - LOG_ERROR_LEVEL
///   - LOG_CRITICAL_LEVEL: only show critical messages
#define LOG_MIN_LEVEL LOG_DEBUG_LEVEL
#endif // LOG_MIN_LEVEL

// For each logging function, we define a corresponding identical macro.
// We can then select the minimum logging severity to log at compile time.

#if LOG_TRACE_LEVEL >= LOG_MIN_LEVEL
#define LOG_TRACE(category, message, ...)                                                                              \
  ::debug::trace((category), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_TRACE(category, message, ...)
#endif

#if LOG_DEBUG_LEVEL >= LOG_MIN_LEVEL
#define LOG_DEBUG(category, message, ...)                                                                              \
  ::debug::debug((category), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_DEBUG(category, message, ...)
#endif

#if LOG_INFO_LEVEL >= LOG_MIN_LEVEL
#define LOG_INFO(category, message, ...)                                                                               \
  ::debug::info((category), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_INFO(category, message, ...)
#endif

#if LOG_WARNING_LEVEL >= LOG_MIN_LEVEL
#define LOG_WARNING(category, message, ...)                                                                            \
  ::debug::warning((category), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_WARNING(category, message, ...)
#endif

#if LOG_ERROR_LEVEL >= LOG_MIN_LEVEL
#define LOG_ERROR(category, message, ...)                                                                              \
  ::debug::error((category), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_ERROR(category, message, ...)
#endif

#if LOG_CRITICAL_LEVEL >= LOG_MIN_LEVEL
#define LOG_CRITICAL(category, message, ...)                                                                           \
  ::debug::critical((category), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_CRITICAL(category, message, ...)
#endif

#define KASSERT(cond) (void)((cond) || (::debug::panic("assertion failed: " #cond), 0))

#endif // OS_DEBUG_HXX
