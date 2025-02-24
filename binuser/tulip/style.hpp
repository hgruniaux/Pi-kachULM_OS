#ifndef TULIP_STYLE_HPP
#define TULIP_STYLE_HPP

#include "utils.hpp"
#include <string.h>

struct TuStyle {
  TuColor text_color;
  TuColor background_color;

  // Button
  TuColor button_color;
  TuColor button_hover_color;
  TuColor button_active_color;

  // Title bar
  TuColor title_text_color;
  TuColor title_text_active_color;
  TuColor title_background_color;
  TuColor title_background_active_color;

  TuSize window_padding;
  TuSize frame_padding;
  uint32_t font_size;

  [[nodiscard]] uint32_t get_text_width(const char* text) const { return font_size * strlen(text); }

  static TuStyle default_light_style();
  static TuStyle default_dark_style();
}; // struct TuStyle

#endif // TULIP_STYLE_HPP
