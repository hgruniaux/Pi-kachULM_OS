#include "graphics.hpp"
#include "../framebuffer.hpp"
#include "pkf.hpp"

extern const uint8_t fonts_myfont_pkf[100];

namespace graphics {
[[gnu::always_inline, nodiscard]] static inline int32_t abs(int32_t x) {
  return x < 0 ? -x : x;
}

template <class T>
[[gnu::always_inline, nodiscard]] static inline T min(T x, T y) {
  return (x < y) ? x : y;
}

template <class T>
[[gnu::always_inline, nodiscard]] static inline T max(T x, T y) {
  return (x < y) ? y : x;
}

/** Rounds @x to the next multiple of @a multiple. */
[[gnu::always_inline, nodiscard]] static uint32_t round_up(uint32_t x, uint32_t multiple) {
  return ((x + multiple - 1) / multiple) * multiple;
}

Painter::Painter() {
  auto& fb = FrameBuffer::get();
  create(fb.get_buffer(), fb.get_width(), fb.get_height(), fb.get_pitch());
}

Painter::Painter(uint32_t* buffer, uint32_t width, uint32_t height, uint32_t pitch) {
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

void Painter::draw_pixel(uint32_t x, uint32_t y) {
  draw_pixel(x, y, m_pen);
}

[[gnu::hot]] void Painter::draw_pixel(uint32_t x, uint32_t y, Color color) {
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
  m_buffer[x + m_pitch * y] = (0xff << 24) | (result_red << 16) | (result_green << 8) | result_blue;
}

void Painter::draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1) {
  draw_line(x0, y0, x1, y1, m_pen);
}

[[gnu::hot]] void Painter::draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, Color color) {
  // See https://en.wikipedia.org/wiki/Bresenham's_line_algorithm

  const int32_t dx = abs((int32_t)x1 - (int32_t)x0);
  const int32_t sx = x0 < x1 ? 1 : -1;
  const int32_t dy = -abs((int32_t)y1 - (int32_t)y0);
  const int32_t sy = y0 < y1 ? 1 : -1;
  int32_t err = dx + dy;

  while (true) {
    draw_pixel(x0, y0, color);
    if (x0 == x1 && y0 == y1)
      break;

    const int32_t e2 = 2 * err;

    if (e2 >= dy) {
      if (x0 == x1)
        break;

      err += dy;
      x0 += sx;
    }

    if (e2 <= dx) {
      if (y0 == y1)
        break;

      err += dx;
      y0 += sy;
    }
  }
}

void Painter::draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
  draw_rect(x, y, w, h, m_pen);
}

[[gnu::hot]] void Painter::draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color color) {
  draw_line(x, y, x + w, y, color);          // top edge
  draw_line(x, y + h, x + w, y + h, color);  // bottom edge
  draw_line(x, y, x, y + h, color);          // left edge
  draw_line(x + w, y, x + w, y + h, color);  // right edge
}

void Painter::fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
  fill_rect(x, y, w, h, m_pen);
}

[[gnu::hot]] void Painter::fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color color) {
  // Early clipping
  x = max(m_clipping.x_min, x);
  y = max(m_clipping.y_min, y);
  w = min(m_clipping.x_max - m_clipping.x_min, w);
  h = min(m_clipping.y_max - m_clipping.y_min, h);

  for (uint32_t i = x; i < (x + w); ++i) {
    for (uint32_t j = y; j < (y + h); ++j) {
      draw_pixel(i, j, color);
    }
  }
}

uint32_t Painter::draw_text(uint32_t x, uint32_t y, const char* text) {
  return draw_text(x, y, UINT32_MAX, text, m_pen);
}

uint32_t Painter::draw_text(uint32_t x, uint32_t y, const char* text, Color color) {
  return draw_text(x, y, UINT32_MAX, text, color);
}

uint32_t Painter::draw_text(uint32_t x, uint32_t y, uint32_t w, const char* text) {
  return draw_text(x, y, w, text, m_pen);
}

uint32_t Painter::draw_text(uint32_t x, uint32_t y, uint32_t w, const char* text, Color color) {
  PKFFile font = PKFFile(fonts_myfont_pkf);

  uint32_t current_x = x;
  uint32_t current_y = y;

  const uint32_t char_width = font.get_char_width();
  const uint32_t char_height = font.get_char_height();
  const uint32_t advance = font.get_horizontal_advance();
  const uint32_t line_height = font.get_line_height();

  for (const char* it = text; *it != '\0'; ++it) {
    const char ch = *it;

    if (ch >= PKFFile::FIRST_CHARACTER && ch <= PKFFile::LAST_CHARACTER) {
      // Early clipping
      if (current_x > m_clipping.x_max)
        continue;

      // If the character does not fit in the line, then start a new line.
      // We check (Y - x >= w) instead of (Y >= x + w) to avoid overflow, as w may be defined to UINT32_MAX.
      if ((current_x + char_width) - x >= w) {
        current_x = x;
        current_y += line_height;
      }

      const uint8_t* glyph = font.get_glyph(ch);
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

[[gnu::always_inline, gnu::hot]] void Painter::draw_alpha_map(uint32_t x,
                                                              uint32_t y,
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

void Painter::revert_clipping() {
  m_clipping.x_min = 0;
  m_clipping.y_min = 0;
  m_clipping.x_max = m_width - 1;
  m_clipping.y_max = m_height - 1;
}

void Painter::set_clipping(uint32_t x_min, uint8_t y_min, uint32_t x_max, uint32_t y_max) {
  m_clipping.x_min = max<uint32_t>(0, x_min);  // maybe unnecessary?
  m_clipping.y_min = max<uint32_t>(0, y_min);  // maybe unnecessary?
  m_clipping.x_max = min(m_width - 1, x_max);
  m_clipping.y_max = min(m_height - 1, y_max);
}
}  // namespace graphics
