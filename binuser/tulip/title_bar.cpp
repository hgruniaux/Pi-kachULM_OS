#include "title_bar.hpp"
#include "painter.hpp"
#include "style.hpp"
#include "window.hpp"

#include <sys/syscall.h>
#include <cstdint>

TuTitleBar::TuTitleBar(TuWidget* parent) : TuWidget(parent) {
  const TuStyle* style = get_style();
  const uint32_t height = style->frame_padding.height * 2 + style->font_size;

  set_min_size({0, height});
  set_max_size({UINT32_MAX, height});
}

void TuTitleBar::on_paint_event(TuPaintEvent* event) {
  TuPainter painter(this);

  TuStyle* style = get_style();
  TuWindow* window = get_window();

  if (window->has_focus()) {
    painter.set_pen(style->title_text_active_color);
    painter.set_brush(style->title_background_active_color);
  } else {
    painter.set_pen(style->title_text_color);
    painter.set_brush(style->title_background_color);
  }

  // Draw background
  painter.fill_rect(event->get_rect());

  // Draw title
  const auto& padding = get_padding();
  painter.draw_text((int32_t)padding.left, (int32_t)padding.top, window->get_title());
}
