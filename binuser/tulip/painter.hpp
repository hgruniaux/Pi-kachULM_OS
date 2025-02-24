#ifndef TULIP_PAINTER_HPP
#define TULIP_PAINTER_HPP

#include "utils.hpp"

class TuWidget;

class TuPainter {
public:
  explicit TuPainter(TuWidget* target);

  [[nodiscard]] TuColor get_pen() const { return m_pen_color; }
  void set_pen(TuColor color) { m_pen_color = color; }

  [[nodiscard]] TuColor get_brush() const { return m_brush_color; }
  void set_brush(TuColor color) { m_brush_color = color; }

  void draw_text(int x, int y, const char* text);
  void draw_text(TuPoint pos, const char* text) {  draw_text(pos.x, pos.y, text); }

  void draw_rect(int x, int y, uint32_t width, uint32_t height);
  void draw_rect(TuRect rect) {  draw_rect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height); }

  void fill_rect(int x, int y, uint32_t width, uint32_t height);
  void fill_rect(TuRect rect) {  fill_rect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height); }

private:
  TuWidget* m_target;
  TuRect m_clip_rect;
  TuColor m_pen_color = {0, 0, 0, 255};
  TuColor m_brush_color = {255, 255, 255, 255};
}; // class TuPainter

#endif // TULIP_PAINTER_HPP
