#pragma once
#include <cstdint>

#include "memory/memory.hpp"

namespace dma_impl {

[[nodiscard]] bool init();

[[nodiscard]] uintptr_t allocate_channel();

void free_channel(uintptr_t chan_base);

void set_channel_enable(uintptr_t chan_base, bool enable);

[[nodiscard]] uintptr_t get_dma_bus_address(VirtualAddress va_addr, bool read_only_address);

};  // namespace dma_impl
