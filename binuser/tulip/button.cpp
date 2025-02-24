#include "button.hpp"
#include <cstdint>
#include "painter.hpp"
#include "style.hpp"

TuButton::TuButton(const char* label, TuWidget* parent) : TuWidget(parent), m_label(label) {
  const uint32_t text_width = get_style()->get_text_width(label);
  set_min_size({text_width + get_padding().left + get_padding().right,
                get_padding().top + +get_padding().bottom + get_style()->font_size});
}

void TuButton::set_label(const char* label) {
  m_label = label;

  const uint32_t text_width = get_style()->get_text_width(label);
  set_min_size({text_width + get_padding().left + get_padding().right,
                get_padding().top + +get_padding().bottom + get_style()->font_size});

  repaint();
}

void TuButton::on_paint_event(TuPaintEvent* event) {
  TuPainter painter(this);

  const TuStyle* style = get_style();

  // Draw the background
  if (m_is_pressed) {
    painter.set_brush(style->button_active_color);
  } else if (m_is_hovered) {
    painter.set_brush(style->button_hover_color);
  } else {
    painter.set_brush(style->button_color);
  }

  painter.fill_rect(event->get_rect());

  // Draw the label
  const uint32_t text_width = style->get_text_width(m_label);

  const int32_t x = (int32_t)((get_width() - text_width) / 2);
  const int32_t y = (int32_t)((get_height() - style->font_size) / 2);

  painter.set_pen(style->text_color);
  painter.draw_text(x, y, m_label);
}
