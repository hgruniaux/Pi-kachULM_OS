#include <libk/assert.hpp>
#include <libk/log.hpp>

namespace libk {
static constexpr size_t MAX_LOGGERS = 5;
static Logger* loggers[MAX_LOGGERS] = {nullptr};

static LogLevel current_log_level = LogLevel::TRACE;

static LogTimer log_timer = nullptr;

namespace detail {
static const char* get_level_string(LogLevel level, bool with_colors) {
  switch (level) {
    case LogLevel::TRACE:
      return "trace";
    case LogLevel::DEBUG:
      return with_colors ? "\x1b[34mdebug\x1b[0m" : "debug";
    case LogLevel::INFO:
      return with_colors ? "\x1b[32minfo\x1b[0m" : "info";
    case LogLevel::WARNING:
      return with_colors ? "\x1b[33mwarning\x1b[0m" : "warning";
    case LogLevel::ERROR:
      return with_colors ? "\x1b[31merror\x1b[0m" : "error";
    case LogLevel::CRITICAL:
      return with_colors ? "\x1b[41mcritical\x1b[0m" : "critical";
  }

  KASSERT(false && "unknown log level");
}

void vlog_logger(Logger* logger,
                 LogLevel level,
                 const char* message,
                 std::source_location source_location,
                 const detail::Argument* args,
                 size_t args_count) {
  static constexpr size_t MAX_BUFFER_SIZE = 1024;

  char buffer[MAX_BUFFER_SIZE];
  char* it = buffer;

  // Print the header
  const char* level_string = get_level_string(level, logger->support_colors());
  if (log_timer != nullptr) {
    it = libk::format_to(it, "[{} ms] ", log_timer());
  }

  if (level < LogLevel::INFO) {
    it = libk::format_to(it, "[{}:{}] ", source_location.file_name(), source_location.line());
  }

  it = libk::format_to(it, "[{}] ", level_string);

  // Print the message itself
  it = detail::format_to(it, message, args, args_count);
  *it = '\0';
  logger->writeln(buffer, (size_t)((intptr_t)it - (intptr_t)buffer));
}

void vlog(LogLevel level,
          const char* message,
          std::source_location source_location,
          const detail::Argument* args,
          size_t args_count) {
  if (level < current_log_level)
    return;

  for (auto* logger : loggers) {
    if (logger == nullptr)
      continue;
    vlog_logger(logger, level, message, source_location, args, args_count);
  }

  if (level >= LogLevel::CRITICAL) {
    panic("A critical log message was emitted", source_location);
  }
}

void vprint(const char* message, const detail::Argument* args, size_t args_count) {
  static constexpr size_t MAX_BUFFER_SIZE = 1024;
  char buffer[MAX_BUFFER_SIZE];

  char* it = detail::format_to(buffer, message, args, args_count);
  *it = '\0';

  for (auto* logger : loggers) {
    if (logger == nullptr)
      continue;

    logger->writeln(buffer, (size_t)((intptr_t)it - (intptr_t)buffer));
  }
}
}  // namespace detail

void set_log_timer(LogTimer timer_in_ms) {
  log_timer = timer_in_ms;
}

void register_logger(Logger& logger) {
  for (auto& entry : loggers) {
    if (entry == nullptr) {
      entry = &logger;
      return;
    }
  }

  KASSERT(false && "too many registered loggers");
}
}  // namespace libk
