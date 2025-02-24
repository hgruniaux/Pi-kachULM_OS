#include "graphics/graphics.hpp"
#include <libk/utils.hpp>
#include <utility>
#include "hardware/framebuffer.hpp"

extern const uint8_t firacode_16_pkf[100];

namespace graphics {
[[gnu::always_inline, nodiscard]] static inline int32_t abs(int32_t x) {
  return x < 0 ? -x : x;
}

Painter::Painter() : m_font(firacode_16_pkf) {
  auto& fb = FrameBuffer::get();
  create(fb.get_buffer(), fb.get_width(), fb.get_height(), fb.get_pitch());
}

Painter::Painter(uint32_t* buffer, uint32_t width, uint32_t height, uint32_t pitch) : m_font(firacode_16_pkf) {
  create(buffer, width, height, pitch);
}

void Painter::create(uint32_t* buffer, uint32_t width, uint32_t height, uint32_t pitch) {
  m_buffer = buffer;
  m_width = width;
  m_height = height;
  m_pitch = pitch;

  revert_clipping();
}

[[gnu::hot]] void Painter::clear(graphics::Color clear_color) {
  for (uint32_t x = 0; x < m_width; ++x) {
    for (uint32_t y = 0; y < m_height; ++y) {
      m_buffer[x + m_pitch * y] = clear_color.argb;
    }
  }
}

void Painter::draw_pixel(int32_t x, int32_t y) {
  draw_pixel(x, y, m_pen);
}

[[gnu::hot]] void Painter::draw_pixel(int32_t x, int32_t y, Color color) {
  // Clipping
  if (x < m_clipping.x_min || x > m_clipping.x_max)
    return;
  if (y < m_clipping.y_min || y > m_clipping.y_max)
    return;

  // Alpha blending: dst = alpha * src + (1 - alpha) * dst
  const uint32_t dst_color = m_buffer[x + m_pitch * y];
  const uint32_t dst_red = (dst_color >> 16) & 0xff;
  const uint32_t dst_green = (dst_color >> 8) & 0xff;
  const uint32_t dst_blue = dst_color & 0xff;

  const uint32_t src_alpha = (color.argb >> 24) & 0xff;
  const uint32_t src_red = (color.argb >> 16) & 0xff;
  const uint32_t src_green = (color.argb >> 8) & 0xff;
  const uint32_t src_blue = color.argb & 0xff;

  const uint32_t result_red = (src_alpha * src_red + (255 - src_alpha) * dst_red) / 255;
  const uint32_t result_green = (src_alpha * src_green + (255 - src_alpha) * dst_green) / 255;
  const uint32_t result_blue = (src_alpha * src_blue + (255 - src_alpha) * dst_blue) / 255;
  m_buffer[x + m_pitch * y] = (0x00 << 24) | (result_red << 16) | (result_green << 8) | result_blue;
}

void Painter::draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
  draw_line(x0, y0, x1, y1, m_pen);
}

[[gnu::hot]] void Painter::draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Color color) {
  // See https://en.wikipedia.org/wiki/Bresenham's_line_algorithm

  const auto dx = x2 - x1;
  const auto dy = y2 - y1;

  // If both of the differences are 0 there will be a division by 0 below.
  if (dx == 0 && dy == 0) {
    draw_pixel(x1, y1, color);
    return;
  }

  if (abs(dx) > abs(dy)) {
    if (x1 > x2) {
      std::swap(x1, x2);
      std::swap(y1, y2);
    }

    for (auto x = x1; x <= x2; ++x) {
      const auto y = dy * (x - x1) / dx + y1;
      draw_pixel(x, y, color);
    }
  } else {
    if (y1 > y2) {
      std::swap(x1, x2);
      std::swap(y1, y2);
    }

    for (auto y = y1; y <= y2; ++y) {
      const auto x = dx * (y - y1) / dy + x1;
      draw_pixel(x, y, color);
    }
  }
}

void Painter::draw_rect(int32_t x, int32_t y, int32_t w, int32_t h) {
  draw_rect(x, y, w, h, m_pen);
}

[[gnu::hot]] void Painter::draw_rect(int32_t x, int32_t y, int32_t w, int32_t h, Color color) {
  draw_line(x, y, x + w - 1, y, color);                  // top edge
  draw_line(x, y + h - 1, x + w - 1, y + h - 1, color);  // bottom edge
  draw_line(x, y, x, y + h - 1, color);                  // left edge
  draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);  // right edge
}

[[gnu::hot]] void Painter::draw_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t line_width, Color color) {
  while (line_width != 0) {
    draw_rect(x, y, w, h, color);
    x += 1;
    y += 1;
    w -= 2;
    h -= 2;
    line_width -= 1;
  }
}

void Painter::fill_rect(int32_t x, int32_t y, int32_t w, int32_t h) {
  fill_rect(x, y, w, h, m_pen);
}

[[gnu::hot]] void Painter::fill_rect(int32_t x, int32_t y, int32_t w, int32_t h, Color color) {
  // Early clipping
  x = libk::max(m_clipping.x_min, x);
  y = libk::max(m_clipping.y_min, y);
  w = libk::min(m_clipping.x_max - m_clipping.x_min + 1, w);
  h = libk::min(m_clipping.y_max - m_clipping.y_min + 1, h);

  for (uint32_t i = x; i < (x + w); ++i) {
    for (uint32_t j = y; j < (y + h); ++j) {
      draw_pixel(i, j, color);
    }
  }
}

uint32_t Painter::draw_text(int32_t x, int32_t y, const char* text) {
  return draw_text(x, y, INT32_MAX, text, m_pen);
}

uint32_t Painter::draw_text(int32_t x, int32_t y, const char* text, Color color) {
  return draw_text(x, y, INT32_MAX, text, color);
}

uint32_t Painter::draw_text(int32_t x, int32_t y, int32_t w, const char* text) {
  return draw_text(x, y, w, text, m_pen);
}

uint32_t Painter::draw_text(int32_t x, int32_t y, int32_t w, const char* text, Color color) {
  auto current_x = x;
  auto current_y = y;

  const uint32_t char_width = m_font.get_char_width();
  const uint32_t char_height = m_font.get_char_height();
  const uint32_t advance = m_font.get_horizontal_advance();
  const uint32_t line_height = m_font.get_line_height();

  for (const char* it = text; *it != '\0'; ++it) {
    const char ch = *it;

    if (ch >= PKFont::FIRST_CHARACTER && ch <= PKFont::LAST_CHARACTER) {
      // Early clipping
      if (current_x > m_clipping.x_max)
        continue;

      // If the character does not fit in the line, then start a new line.
      // We check (Y - x >= w) instead of (Y >= x + w) to avoid overflow, as w may be defined to UINT32_MAX.
      if ((current_x + char_width) - x >= w) {
        current_x = x;
        current_y += line_height;
      }

      const uint8_t* glyph = m_font.get_glyph(ch);
      draw_alpha_map(current_x, current_y, glyph, char_width, char_height, color);
      current_x += advance;
    } else {
      switch (ch) {
        case ' ':
          current_x += advance;
          break;
        case '\n':
          current_x = x;
          current_y += line_height;
          break;
        default:
          // Ignore the unknown character.
          // We aim that this text rendered is straightforward.
          // So, we don't handle tabulations, backspaces, or any advanced controls.
          break;
      }
    }

    // Early clipping
    if (current_y > m_clipping.y_max)
      return current_x;
  }

  return current_x;
}

[[gnu::hot]] void Painter::draw_alpha_map(int32_t x,
                                          int32_t y,
                                          const uint8_t* alpha_map,
                                          uint32_t w,
                                          uint32_t h,
                                          Color color) {
  // This function is a performance bottleneck.
  // It is called to draw each glyph.
  // Therefore, it must be heavily optimized if possible.
  for (uint32_t i = 0; i < w; ++i) {
    for (uint32_t j = 0; j < h; ++j) {
      const uint8_t alpha = alpha_map[i + j * w];
      draw_pixel(x + i, y + j, color.with_alpha(alpha));
    }
  }
}

void Painter::blit(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint32_t* pixels_buffer) {
  // TODO: Clipping
  for (uint32_t i = 0; i < width; ++i) {
    for (uint32_t j = 0; j < height; ++j) {
      m_buffer[(x + i) + m_pitch * (y + j)] = pixels_buffer[i + width * j];
    }
  }
}

void Painter::revert_clipping() {
  m_clipping.x_min = 0;
  m_clipping.y_min = 0;
  m_clipping.x_max = m_width - 1;
  m_clipping.y_max = m_height - 1;
}

void Painter::set_clipping(int32_t x_min, int32_t y_min, int32_t x_max, int32_t y_max) {
  m_clipping.x_min = libk::max<int32_t>(0, x_min);  // maybe unnecessary?
  m_clipping.y_min = libk::max<int32_t>(0, y_min);  // maybe unnecessary?
  m_clipping.x_max = libk::min<int32_t>(m_width - 1, x_max);
  m_clipping.y_max = libk::min<int32_t>(m_height - 1, y_max);
}
}  // namespace graphics
