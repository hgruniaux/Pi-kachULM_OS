// Author: Hubert Gruniaux
// Date: 2/17/24
// The following code is released in the public domain (where applicable).

#ifndef OS_DEBUG_HXX
#define OS_DEBUG_HXX

#include <source_location>

namespace debug {
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
log(Severity severity,
    const char* category,
    const char* message,
    std::source_location source_location = std::source_location::current());

inline void
trace(const char* category,
      const char* message,
      std::source_location source_location = std::source_location::current())
{
  log(Severity::TRACE, category, message, source_location);
}

inline void
debug(const char* category,
      const char* message,
      std::source_location source_location = std::source_location::current())
{
  log(Severity::DEBUG, category, message, source_location);
}

inline void
info(const char* category,
     const char* message,
     std::source_location source_location = std::source_location::current())
{
  log(Severity::INFO, category, message, source_location);
}

inline void
warning(const char* category,
        const char* message,
        std::source_location source_location = std::source_location::current())
{
  log(Severity::WARNING, category, message, source_location);
}

inline void
error(const char* category,
      const char* message,
      std::source_location source_location = std::source_location::current())
{
  log(Severity::ERROR, category, message, source_location);
}

inline void
critical(const char* category,
         const char* message,
         std::source_location source_location = std::source_location::current())
{
  log(Severity::CRITICAL, category, message, source_location);
}

void
panic(const char* message,
      std::source_location source_location = std::source_location::current());
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
#define LOG_MIN_LEVEL LOG_CRITICAL_LEVEL
#endif // LOG_MIN_LEVEL

// For each logging function, we define a corresponding identical macro.
// We can then select the minimum logging severity to log at compile time.

#if LOG_TRACE_LEVEL >= LOG_MIN_LEVEL
#define LOG_TRACE(category, message) ::debug::trace((category), (message))
#else
#define LOG_TRACE(category, message)
#endif

#if LOG_DEBUG_LEVEL >= LOG_MIN_LEVEL
#define LOG_DEBUG(category, message) ::debug::debug((category), (message))
#else
#define LOG_DEBUG(category, message)
#endif

#if LOG_INFO_LEVEL >= LOG_MIN_LEVEL
#define LOG_INFO(category, message) ::debug::info((category), (message))
#else
#define LOG_INFO(category, message)
#endif

#if LOG_WARNING_LEVEL >= LOG_MIN_LEVEL
#define LOG_WARNING(category, message) ::debug::warning((category), (message))
#else
#define LOG_WARNING(category, message)
#endif

#if LOG_ERROR_LEVEL >= LOG_MIN_LEVEL
#define LOG_ERROR(category, message) ::debug::error((category), (message))
#else
#define LOG_ERROR(category, message)
#endif

#if LOG_CRITICAL_LEVEL >= LOG_MIN_LEVEL
#define LOG_CRITICAL(category, message) ::debug::critical((category), (message))
#else
#define LOG_CRITICAL(category, message)
#endif

#define KASSERT(cond) (void)((cond) || (::debug::panic("assertion failed: " #cond), 0))

#endif // OS_DEBUG_HXX
