#pragma once

#include "ffconf.h"

inline static constexpr PhysicalPA RAM_FS_PHYSICAL_LOAD_ADDRESS = 0x18000000;
inline static constexpr size_t RAM_FS_BYTE_SIZE = 0xa00000;  // 10 Mio (Must be a multiple of PAGE_SIZE)
inline static constexpr size_t RAM_FS_SECTOR_COUNT = RAM_FS_BYTE_SIZE / FF_MIN_SS;
