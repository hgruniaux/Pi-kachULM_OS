#include "system_timer.hpp"
#include <libk/utils.hpp>
#include <limits>
#include "hardware/irq/irq_lists.hpp"
#include "hardware/irq/irq_manager.hpp"
#include "hardware/kernel_dt.hpp"
#include "libk/log.hpp"

namespace SystemTimer {
constexpr uint32_t CS = 0x00;   // System Timer Control/Status
constexpr uint32_t CLO = 0x04;  // System Timer Counter Lower 32 bits
constexpr uint32_t CHI = 0x08;  // System Timer Counter Higher 32 bits
constexpr uint32_t CB = 0x0c;   // System Timer Compare Base
//  CB +  0 = 0x0c, System Timer Compare 0
//  CB +  4 = 0x10, System Timer Compare 1
//  CB +  8 = 0x14, System Timer Compare 2
//  CB + 12 = 0x14, System Timer Compare 3

constexpr uint64_t freq = 1'000'000;  // System Timer Frequency

static uintptr_t _base;
static bool _tim_reserved[nb_timers] = {false};
static bool _tim_used[nb_timers] = {false};
static CallBack _callbacks[nb_timers] = {nullptr};
static uint64_t _tick_period[nb_timers] = {0};

void init_base() {
  if (KernelDT::get_device_address("system_timer", &_base)) {
    return;
  }

  for (const auto& compatible : KernelDT::get_board_compatible()) {
    if (compatible.find("bcm2837") != libk::StringView::npos) {
      _base = KernelDT::convert_soc_address(0x7E003000);
      return;
    }
  }

  libk::panic("No system timer found.");
}

void setup_next_period(size_t tim, uint32_t period) {
  const uint64_t current_tick = get_tick_count();
  const uint32_t next_counter_match = (current_tick & std::numeric_limits<uint32_t>::max()) + period;
  libk::write32(_base + CB + tim * sizeof(uint32_t), next_counter_match);
}

void process_interrupt(void* handle) {
  const size_t timer = (uintptr_t)handle;
  const uint32_t mask = (uint32_t)1 << timer;
  const uint32_t timer_matchs = libk::read32(_base + CS);

  if ((timer_matchs & mask) == 0) {
    return;
  }

  const CallBack cb = _callbacks[timer];
  const uint32_t period = _tick_period[timer];
  if (period != 0) {
    setup_next_period(timer, period);
  }
  libk::write32(_base + CS, mask);

  if (cb != nullptr) {
    (*cb)();
  }
}

void init() {
  init_base();

  for (size_t tim = 0; tim < nb_timers; ++tim) {
    _tim_reserved[tim] = libk::read32(_base + CB + tim * sizeof(uint32_t)) != 0;

    if (!_tim_reserved[tim]) {
      IRQManager::register_irq_handler({.type = VC_TIMER_BASE.type, .id = VC_TIMER_BASE.id + tim},
                                               &process_interrupt, (void*)tim);
    }
  }
}

void setup_next_period(size_t tim, uint32_t period);

uint64_t get_frequency() {
  return freq;
}

uint64_t get_tick_count() {
  return libk::read32(_base + CLO) + ((uint64_t)libk::read32(_base + CHI) << 32);
}

uint64_t get_elapsed_time_in_ns() {
  return (get_tick_count() * 1'000'000'000) / freq;
}
uint64_t get_elapsed_time_in_micros() {
  return (get_tick_count() * 1'000'000) / freq;
}
uint64_t get_elapsed_time_in_ms() {
  return (get_tick_count() * 1'000) / get_frequency();
}
uint64_t get_elapsed_time_in_s() {
  return get_tick_count() / get_frequency();
}

bool is_reserved(size_t tim) {
  if (tim >= nb_timers) {
    return true;
  }

  return _tim_reserved[tim];
}

bool is_used(size_t tim) {
  if (tim >= nb_timers) {
    return true;
  }

  return is_reserved(tim) || _tim_used[tim];
}

bool set_recurrent_tick(size_t tim, uint32_t tick_period, CallBack callback) {
  if (is_reserved(tim)) {
    return false;
  }

  _tick_period[tim] = tick_period;
  _callbacks[tim] = callback;
  _tim_used[tim] = true;

  setup_next_period(tim, tick_period);
  return true;
}
bool set_recurrent_micros(size_t tim, uint32_t micros_period, CallBack callback) {
  return set_recurrent_tick(tim, micros_period * freq / 1'000'000, callback);
}
bool set_recurrent_ms(size_t tim, uint32_t ms_period, CallBack callback) {
  return set_recurrent_tick(tim, ms_period * freq / 1'000, callback);
}
bool set_recurrent_s(size_t tim, uint32_t s_period, CallBack callback) {
  return set_recurrent_tick(tim, s_period * freq, callback);
}

bool set_oneshot_tick(size_t tim, uint32_t tick_duration, CallBack callback) {
  if (is_reserved(tim)) {
    return false;
  }

  _tick_period[tim] = 0;
  _callbacks[tim] = callback;
  _tim_used[tim] = true;

  setup_next_period(tim, tick_duration);
  return true;
}
bool set_oneshot_micros(size_t tim, uint32_t micro_duration, CallBack callback) {
  return set_oneshot_tick(tim, micro_duration * freq / 1'000'000, callback);
}
bool set_oneshot_ms(size_t tim, uint32_t ms_duration, CallBack callback) {
  return set_oneshot_tick(tim, ms_duration * freq / 1'000, callback);
}
bool set_oneshot_s(size_t tim, uint32_t s_duration, CallBack callback) {
  return set_oneshot_tick(tim, s_duration * freq, callback);
}

}  // namespace SystemTimer
