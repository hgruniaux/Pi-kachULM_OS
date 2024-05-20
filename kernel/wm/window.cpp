#include "window.hpp"
#include "memory/mem_alloc.hpp"
#include "pika_icon.hpp"

Window::Window(const libk::SharedPointer<Task>& task) : m_task(task) {
  KASSERT(task != nullptr);

  m_painter = {nullptr, 0, 0, 0};
}

void Window::set_title(libk::StringView title) {
  delete[] m_title.get_data();

  // Clamp the title length, so the user cannot abuse from the kernel.
  const size_t length = libk::min(title.get_length(), MAX_TITLE_LENGTH);

  char* buffer = new char[length + 1];
  libk::memcpy(buffer, title.get_data(), length);
  buffer[length] = '\0';
  m_title = {buffer, length};
}

void Window::set_geometry(const Rect& rect) {
  const bool resized = rect.width() != m_geometry.width() || rect.height() != m_geometry.height();

  m_geometry = rect;

  // Reallocate the framebuffer if needed.
  if (resized)
    reallocate_framebuffer();
}

void Window::clear(uint32_t argb) {
  m_painter.clear(argb);
}

void Window::draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t argb) {
  m_painter.draw_line(x1, y1, x2, y2, argb);
}

void Window::draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t argb) {
  m_painter.draw_rect(x, y, width, height, argb);
}

void Window::fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t argb) {
  m_painter.fill_rect(x, y, width, height, argb);
}

void Window::draw_text(uint32_t x, uint32_t y, const char* text, uint32_t argb) {
  m_painter.draw_text(x, y, text, argb);
}

void Window::draw_frame() {
  if (!m_has_frame)
    return;

  constexpr const uint32_t TITLE_BAR_HEIGHT = 30;
  constexpr const graphics::Color BACKGROUND_COLOR = 0xff282828;
  constexpr const graphics::Color BORDER_COLOR = 0xff121212;

  m_painter.fill_rect(0, 0, m_geometry.width(), TITLE_BAR_HEIGHT, BACKGROUND_COLOR);
  m_painter.draw_rect(0, 0, m_geometry.width(), m_geometry.height(), BORDER_COLOR);
  m_painter.draw_line(0, TITLE_BAR_HEIGHT, m_geometry.width(), TITLE_BAR_HEIGHT, BORDER_COLOR);

#if 1
  // Draw the pikachu icon.
  constexpr uint32_t pika_color = 0xffffff;
  for (uint32_t i = 0; i < pika_icon_width; ++i) {
    for (uint32_t j = 0; j < pika_icon_height; ++j) {
      const uint32_t color = pika_color | (pika_icon[i + pika_icon_height * j] << 24);
      m_painter.draw_pixel(5 + i, 2 + j, color);
    }
  }
#endif

  const auto text_width = m_painter.get_font().get_width(m_title.get_data(), m_title.get_length());
  const auto text_x = (m_geometry.width() - text_width) / 2;
  const auto text_y = (TITLE_BAR_HEIGHT - m_painter.get_font().get_char_height()) / 2;
  m_painter.draw_text(text_x, text_y, m_title.get_data(), 0xffffff);
}

void Window::reallocate_framebuffer() {
  delete[] m_framebuffer;
  m_framebuffer = new uint32_t[m_geometry.width() * m_geometry.height()];
  m_framebuffer_pitch = m_geometry.width();
  libk::bzero(m_framebuffer, sizeof(uint32_t) * m_geometry.width() * m_geometry.height());
  m_painter = graphics::Painter(m_framebuffer, m_geometry.width(), m_geometry.height(), m_framebuffer_pitch);
}
