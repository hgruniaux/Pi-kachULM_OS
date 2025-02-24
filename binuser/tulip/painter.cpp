#include "painter.hpp"
#include "widget.hpp"
#include "window.hpp"

#include <sys/window.h>
#include <sys/syscall.h>

TuPainter::TuPainter(TuWidget* target) : m_target(target) {
  m_clip_rect = { { 0, 0 }, target->get_size() };
}

void TuPainter::draw_text(int x, int y, const char* text) {
  // TODO: Support clipping
  // TODO: What to do when global_x/y is negative?
  sys_window_t* window_handle = m_target->get_window()->get_handle();
  const int32_t global_x = m_target->get_x() + x;
  const int32_t global_y = m_target->get_y() + y;
  sys_gfx_draw_text(window_handle, global_x, global_y, text, m_pen_color.to_uint32());
}

void TuPainter::draw_rect(int x, int y, uint32_t width, uint32_t height) {
  // TODO: Support clipping
  // TODO: What to do when global_x/y is negative?
  sys_window_t* window_handle = m_target->get_window()->get_handle();
  const int32_t global_x = m_target->get_x() + x;
  const int32_t global_y = m_target->get_y() + y;
  sys_gfx_draw_rect(window_handle, global_x, global_y, width, height, m_pen_color.to_uint32());
}

void TuPainter::fill_rect(int x, int y, uint32_t width, uint32_t height) {
  // TODO: Support clipping
  // TODO: What to do when global_x/y is negative?
  sys_window_t* window_handle = m_target->get_window()->get_handle();
  const int32_t global_x = m_target->get_x() + x;
  const int32_t global_y = m_target->get_y() + y;
  sys_gfx_fill_rect(window_handle, global_x, global_y, width, height, m_brush_color.to_uint32());
}
