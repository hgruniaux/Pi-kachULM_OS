#pragma once

#include <cstddef>
#include <cstdint>
#include <source_location>

#include "libk/string_view.hpp"

/** See debug::Severity::TRACE. */
#define LOG_TRACE_LEVEL 0
/** @brief See debug::Severity::DEBUG. */
#define LOG_DEBUG_LEVEL 1
/** @brief See debug::Severity::INFO. */
#define LOG_INFO_LEVEL 2
/** @brief See debug::Severity::WARNING. */
#define LOG_WARNING_LEVEL 3
/** @brief See debug::Severity::ERROR. */
#define LOG_ERROR_LEVEL 4
/** @brief See debug::Severity::CRITICAL. */
#define LOG_CRITICAL_LEVEL 5

namespace debug {
struct Logger {
  const char* name;
};  // struct Logger

/** Declares a new debug logger.
 * Can be called many times, however DEBUG_IMPL_LOGGER must be called at least once and no
 * more than once. */
#define DEBUG_DECL_LOGGER(name) extern ::debug::Logger name;
/** Implements a new debug logger. To be called once, in a source file. */
#define DEBUG_IMPL_LOGGER(name) ::debug::Logger name = {#name};

// Declares the default debug logger.
DEBUG_DECL_LOGGER(default_logger);

namespace impl {
/** @brief A tagged-union that store an argument to a log message. */
struct Argument {
  /** @brief The different supported argument types. */
  enum class Type : uint8_t {
    /** @brief The `bool` type. */
    BOOL,
    /** @brief The `char` type. */
    CHAR,
    /** @brief The `int64_t` type. */
    INT64,
    /** @brief The `uint64_t` type. */
    UINT64,
    /** @brief The `float` type. */
    //    FLOAT,
    /** @brief The `double` type. */
    //    DOUBLE,
    /** @brief A pointer. */
    POINTER,
    /** @brief A NUL-terminated UTF-8 string. */
    C_STRING,
    /** @brief A sized UTF-8 string. */
    C_SIZED_STRING,
  };

  Type type;
  union {
    bool bool_value;
    char char_value;
    int64_t int64_value;
    uint64_t uint64_value;
    float float_value;
    double double_value;
    const char* c_string_value;
    const void* pointer_value;
    struct {
      const char* data;
      size_t length;
    } c_sized_string_value;
  } data;

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
  //  Argument(float value) : type(Type::FLOAT) { data.float_value = value; }
  //  Argument(double value) : type(Type::DOUBLE) { data.double_value = value; }
  Argument(const char* value) : type(Type::C_STRING) { data.c_string_value = value; }
  Argument(const void* value) : type(Type::POINTER) { data.pointer_value = value; }
  Argument(libk::StringView value) : type(Type::C_SIZED_STRING) {
    data.c_sized_string_value.length = value.get_length();
    data.c_sized_string_value.data = value.get_data();
  }
};  // class Argument
}  // namespace impl

/** @brief The different log levels supported by the debug module. */
enum class Level : uint8_t {
  TRACE = LOG_TRACE_LEVEL,
  DEBUG = LOG_DEBUG_LEVEL,
  INFO = LOG_INFO_LEVEL,
  WARNING = LOG_WARNING_LEVEL,
  ERROR = LOG_ERROR_LEVEL,
  CRITICAL = LOG_CRITICAL_LEVEL
};  // enum class Severity

namespace impl {
void vprint(const char* message, const impl::Argument* args, size_t args_count);

void vlog(Level level,
          const Logger& logger,
          const char* message,
          std::source_location source_location,
          const impl::Argument* args,
          size_t args_count);
}  // namespace impl

template <class... Args>
inline void print(const char* message, const Args&... args) {
  impl::Argument args_array[] = {args...};
  impl::vprint(message, args_array, sizeof...(Args));
}

/** @brief Logs a formatted message in the kernel logs.
 *
 * Prefer to use the LOG_*() macros. */
template <class... Args>
inline void log(Level level,
                const Logger& logger,
                const char* message,
                std::source_location source_location,
                const Args&... args) {
  impl::Argument args_array[] = {args...};
  impl::vlog(level, logger, message, source_location, args_array, sizeof...(Args));
}

/** @brief Same as log() but uses a default logger. */
template <class... Args>
inline void log(Level severity, const char* message, std::source_location source_location, const Args&... args) {
  log(severity, default_logger, message, source_location, args...);
}

[[noreturn]] void panic(const char* message, std::source_location source_location = std::source_location::current());
}  // namespace debug

#ifndef LOG_MIN_LEVEL
/** LOG_MIN_LEVEL defines the minimum severity level to be logged.
 * It can be defined to one of:
 *   - LOG_DEBUG_LEVEL: show all messages including debug ones
 *   - LOG_INFO_LEVEL
 *   - LOG_WARNING_LEVEL
 *   - LOG_ERROR_LEVEL
 *   - LOG_CRITICAL_LEVEL: only show critical messages */
#define LOG_MIN_LEVEL LOG_INFO_LEVEL
#endif  // LOG_MIN_LEVEL

// For each logging function, we define a corresponding identical macro.
// We can then select the minimum logging severity to log at compile time.

#if LOG_TRACE_LEVEL >= LOG_MIN_LEVEL
#define LOG_TRACE_WITH_LOGGER(logger, message, ...) \
  ::debug::log(::debug::Level::TRACE, (logger), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#define LOG_TRACE(message, ...) LOG_TRACE_WITH_LOGGER(::debug::default_logger, message __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_TRACE_WITH_LOGGER(logger, message, ...)
#define LOG_TRACE(message, ...)
#endif

#if LOG_DEBUG_LEVEL >= LOG_MIN_LEVEL
#define LOG_DEBUG_WITH_LOGGER(logger, message, ...) \
  ::debug::log(::debug::Level::DEBUG, (logger), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#define LOG_DEBUG(message, ...) LOG_DEBUG_WITH_LOGGER(::debug::default_logger, message __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_DEBUG_WITH_LOGGER(logger, message, ...)
#define LOG_DEBUG(message, ...)
#endif

#if LOG_INFO_LEVEL >= LOG_MIN_LEVEL
#define LOG_INFO_WITH_LOGGER(logger, message, ...) \
  ::debug::log(::debug::Level::INFO, (logger), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#define LOG_INFO(message, ...) LOG_INFO_WITH_LOGGER(::debug::default_logger, message __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_INFO_WITH_LOGGER(logger, message, ...)
#define LOG_INFO(message, ...)
#endif

#if LOG_WARNING_LEVEL >= LOG_MIN_LEVEL
#define LOG_WARNING_WITH_LOGGER(logger, message, ...) \
  ::debug::log(::debug::Level::WARNING, (logger), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARNING(message, ...) LOG_WARNING_WITH_LOGGER(::debug::default_logger, message __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_WARNING_WITH_LOGGER(logger, message, ...)
#define LOG_WARNING(message, ...)
#endif

#if LOG_ERROR_LEVEL >= LOG_MIN_LEVEL
#define LOG_ERROR_WITH_LOGGER(logger, message, ...) \
  ::debug::log(::debug::Level::ERROR, (logger), (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#define LOG_ERROR(message, ...) LOG_ERROR_WITH_LOGGER(::debug::default_logger, message __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_ERROR_WITH_LOGGER(logger, message, ...)
#define LOG_ERROR(message, ...)
#endif

#if LOG_CRITICAL_LEVEL >= LOG_MIN_LEVEL
#define LOG_CRITICAL_WITH_LOGGER(logger, message, ...)        \
  ::debug::log(::debug::Level::CRITICAL, (logger), (message), \
               std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#define LOG_CRITICAL(message, ...) LOG_CRITICAL_WITH_LOGGER(::debug::default_logger, message __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_CRITICAL_WITH_LOGGER(logger, message, ...)
#define LOG_CRITICAL(message, ...)
#endif

#ifndef KASSERT
#define KASSERT(cond) (void)((cond) || (::debug::panic("assertion failed: " #cond, std::source_location::current()), 0))
#endif  // KASSERT
