#pragma once

#ifdef CONFIG_INCLUDE_WALLPAPER
#include <cstddef>
#include <cstdint>

// The wallpaper is encoded as a JPG image.
extern "C" size_t wallpaper_jpg_len;
extern "C" const uint8_t wallpaper_jpg[];
#endif  // CONFIG_INCLUDE_WALLPAPER
