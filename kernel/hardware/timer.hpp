#pragma once

#include <cstdint>

class GenericTimer {
 public:
  /** @brief Returns the timer frequency in Hertz. */
  [[nodiscard]] static inline uint64_t get_frequency() {
    uint64_t freq;
    asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(freq));
    return freq;
  }

  /** @brief Returns the timer tick count since boot. */
  [[nodiscard]] static inline uint64_t get_tick_count() {
    uint64_t count;
    asm volatile("mrs %0, CNTPCT_EL0" : "=r"(count));
    return count;
  }

  /** @brief Returns elapsed time (in nanoseconds) since boot. */
  [[nodiscard]] static inline uint64_t get_elapsed_time_in_ns() {
    return (get_tick_count() * 1'000'000'000) / get_frequency();
  }
  /** @brief Returns elapsed time (in microseconds) since boot. */
  [[nodiscard]] static inline uint64_t get_elapsed_time_in_micros() {
    return (get_tick_count() * 1'000'000) / get_frequency();
  }
  /** @brief Returns elapsed time (in milliseconds) since boot. */
  [[nodiscard]] static inline uint64_t get_elapsed_time_in_ms() { return (get_tick_count() * 1'000) / get_frequency(); }
  /** @brief Returns elapsed time (in seconds) since boot. */
  [[nodiscard]] static inline uint64_t get_elapsed_time_in_s() { return get_tick_count() / get_frequency(); }
};  // class GenericTimer
