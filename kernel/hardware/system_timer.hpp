#pragma once

#include <cstddef>
#include <cstdint>

namespace SystemTimer {
/** This is the number of timers available. */
static inline constexpr size_t nb_timers = 4;
using CallBack = void (*)();

void init();

/** @brief Returns the system timer frequency in Hertz. */
[[nodiscard]] uint64_t get_frequency();

/** @brief Returns the system timer tick count since boot. */
[[nodiscard]] uint64_t get_tick_count();

/** @brief Returns the system timer time (in nanoseconds) since boot. */
[[nodiscard]] uint64_t get_elapsed_time_in_ns();
/** @brief Returns the system timer time (in microseconds) since boot. */
[[nodiscard]] uint64_t get_elapsed_time_in_micros();
/** @brief Returns the system timer time (in milliseconds) since boot. */
[[nodiscard]] uint64_t get_elapsed_time_in_ms();
/** @brief Returns the system timer time (in seconds) since boot. */
[[nodiscard]] uint64_t get_elapsed_time_in_s();

/** @brief Returns if the timer @a tim is reserved by the firmware. */
[[nodiscard]] bool is_reserved(size_t tim);

/** @brief Returns if the timer @a tim is used. If not, it's free to use !. */
[[nodiscard]] bool is_used(size_t tim);

/** @brief Setup the @a tim timer to cause in interrupt each @a tick_period ticks to call @a callback. */
[[nodiscard]] bool set_recurrent_tick(size_t tim, uint32_t tick_period, CallBack callback);
/** @brief Setup the @a tim timer to cause in interrupt each @a micro_period microseconds to call @a callback. */
[[nodiscard]] bool set_recurrent_micros(size_t tim, uint32_t micro_period, CallBack callback);
/** @brief Setup the @a tim timer to cause in interrupt each @a ms_period milliseconds to call @a callback. */
[[nodiscard]] bool set_recurrent_ms(size_t tim, uint32_t ms_period, CallBack callback);
/** @brief Setup the @a tim timer to cause in interrupt each @a s_period seconds to call @a callback. */
[[nodiscard]] bool set_recurrent_s(size_t tim, uint32_t s_period, CallBack callback);

/** @brief Setup the @a tim timer to cause in interrupt in @a tick_duration ticks to call @a callback. */
[[nodiscard]] bool set_oneshot_tick(size_t tim, uint32_t tick_duration, CallBack callback);
/** @brief Setup the @a tim timer to cause in interrupt in @a micro_duration microseconds to call @a callback. */
[[nodiscard]] bool set_oneshot_micros(size_t tim, uint32_t micro_duration, CallBack callback);
/** @brief Setup the @a tim timer to cause in interrupt in @a ms_duration milliseconds to call @a callback. */
[[nodiscard]] bool set_oneshot_ms(size_t tim, uint32_t ms_duration, CallBack callback);
/** @brief Setup the @a tim timer to cause in interrupt in @a s_duration seconds to call @a callback. */
[[nodiscard]] bool set_oneshot_s(size_t tim, uint32_t s_duration, CallBack callback);

};  // namespace SystemTimer
