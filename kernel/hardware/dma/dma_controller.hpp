#pragma once

#include "memory/memory.hpp"

namespace DMA {
using Address = uint32_t;

[[nodiscard]] bool init();

/** Convert a KERNEL virtual address to its corresponding DMA address. */
[[nodiscard]] Address get_dma_bus_address(VirtualAddress va_addr, bool read_only_address);

}  // namespace DMA
