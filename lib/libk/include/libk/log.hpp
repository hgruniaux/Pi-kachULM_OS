#pragma once

#include <cstddef>
#include <source_location>
#include "format.hpp"

/** See debug::Severity::TRACE. */
#define LOG_TRACE_LEVEL 0
/** @brief See debug::Severity::libk. */
#define LOG_DEBUG_LEVEL 1
/** @brief See debug::Severity::INFO. */
#define LOG_INFO_LEVEL 2
/** @brief See debug::Severity::WARNING. */
#define LOG_WARNING_LEVEL 3
/** @brief See debug::Severity::ERROR. */
#define LOG_ERROR_LEVEL 4
/** @brief See debug::Severity::CRITICAL. */
#define LOG_CRITICAL_LEVEL 5

namespace libk {
class Logger {
 public:
  /** Returns true if ANSI escape sequences should be emitted. */
  [[nodiscard]] virtual bool support_colors() const { return false; }
  /** Writes the given UTF-8 encoded data to the output. */
  virtual void writeln(const char* data, size_t length) = 0;
};  // class Logger

/** @brief The different log levels supported by the debug module. */
enum class LogLevel : uint8_t {
  TRACE = LOG_TRACE_LEVEL,
  DEBUG = LOG_DEBUG_LEVEL,
  INFO = LOG_INFO_LEVEL,
  WARNING = LOG_WARNING_LEVEL,
  ERROR = LOG_ERROR_LEVEL,
  CRITICAL = LOG_CRITICAL_LEVEL
};  // enum class Severity

[[nodiscard]] inline bool operator<(LogLevel lhs, LogLevel rhs) {
  return (uint8_t)lhs < (uint8_t)rhs;
}

namespace detail {
void vlog(LogLevel level,
          const char* message,
          std::source_location source_location,
          const detail::Argument* args,
          size_t args_count);

void vprint(const char* message, const detail::Argument* args, size_t args_count);
}  // namespace detail

void register_logger(Logger& logger);

using LogTimer = uint64_t (*)();
void set_log_timer(LogTimer timer_in_ms);

/** @brief Logs a formatted message in the kernel logs.
 *
 * Prefer to use the LOG_*() macros. */
template <class... Args>
inline void log(LogLevel level, const char* message, std::source_location source_location, const Args&... args) {
  detail::Argument args_array[] = {args...};
  detail::vlog(level, message, source_location, args_array, sizeof...(Args));
}

template <class... Args>
inline void print(const char* message, const Args&... args) {
  detail::Argument args_array[] = {args...};
  detail::vprint(message, args_array, sizeof...(Args));
}
}  // namespace libk

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
#define LOG_TRACE(message, ...) \
  ::libk::log(::libk::LogLevel::TRACE, (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_TRACE(message, ...)
#endif

#if LOG_DEBUG_LEVEL >= LOG_MIN_LEVEL
#define LOG_DEBUG(message, ...) \
  ::libk::log(::libk::LogLevel::libk, (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_DEBUG(message, ...)
#endif

#if LOG_INFO_LEVEL >= LOG_MIN_LEVEL
#define LOG_INFO(message, ...) \
  ::libk::log(::libk::LogLevel::INFO, (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_INFO(message, ...)
#endif

#if LOG_WARNING_LEVEL >= LOG_MIN_LEVEL
#define LOG_WARNING(message, ...) \
  ::libk::log(::libk::LogLevel::WARNING, (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_WARNING(message, ...)
#endif

#if LOG_ERROR_LEVEL >= LOG_MIN_LEVEL
#define LOG_ERROR(message, ...) \
  ::libk::log(::libk::LogLevel::ERROR, (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_ERROR(message, ...)
#endif

#if LOG_CRITICAL_LEVEL >= LOG_MIN_LEVEL
#define LOG_CRITICAL(message, ...) \
  ::libk::log(::libk::LogLevel::CRITICAL, (message), std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)
#else
#define LOG_CRITICAL(message, ...)
#endif
