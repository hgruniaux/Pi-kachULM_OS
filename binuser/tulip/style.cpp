#include "style.hpp"

TuStyle TuStyle::default_light_style() {
  // TODO
  return {};
}

TuStyle TuStyle::default_dark_style() {
  TuStyle style;
  style.text_color = {255, 255, 255, 255};
  style.background_color = {15, 15, 15, 255};

  // Button
  style.button_color = {50, 50, 50, 255};
  style.button_hover_color = {70, 70, 70, 255};
  style.button_active_color = {0, 110, 180, 255};

  // Title bar
  style.title_text_color = {230, 230, 230, 255};
  style.title_text_active_color = {255, 255, 255, 255};
  style.title_background_color = {25, 25, 25, 255};
  style.title_background_active_color = {0, 110, 180, 255};

  style.frame_padding = {9, 9};
  style.window_padding = {11, 11};
  style.font_size = 16;

  return style;
}
