#pragma once

#include <cstdint>

namespace PS2Keyboard {
using Event = void (*)(uint64_t);

void init();

void set_on_event(Event ev);

void set_sticky_keys(bool enable);
};  // namespace PS2Keyboard
