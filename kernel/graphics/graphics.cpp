#include "graphics.hpp"
#include "../framebuffer.hpp"
#include "pkf.hpp"

extern const uint8_t fonts_myfont_pkf[100];

namespace graphics {
void clear(Color color) {
  FrameBuffer::get().clear(color.argb);
}

void draw_pixel(uint32_t x, uint32_t y, Color color) {
  auto& fb = FrameBuffer::get();
  if (x >= fb.get_width() || y >= fb.get_height())
    return;

  // Alpha blending:
  const uint32_t dst_color = fb.get_pixel(x, y);
  const uint32_t dst_red = (dst_color >> 16) & 0xff;
  const uint32_t dst_green = (dst_color >> 8) & 0xff;
  const uint32_t dst_blue = dst_color & 0xff;

  const uint32_t src_alpha = (color.argb >> 24) & 0xff;
  const uint32_t src_red = (color.argb >> 16) & 0xff;
  const uint32_t src_green = (color.argb >> 8) & 0xff;
  const uint32_t src_blue = color.argb & 0xff;

  // dst = alpha * src + (1 - alpha) * dst
  fb.set_pixel(x, y, (src_alpha * src_red + (255 - src_alpha) * dst_red) / 255,
               (src_alpha * src_green + (255 - src_alpha) * dst_green) / 255,
               (src_alpha * src_blue + (255 - src_alpha) * dst_blue) / 255);
}

int32_t abs(int32_t x) {
  return x < 0 ? -x : x;
}

void draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, Color color) {
  // This function implements the Bresenham's line algorithm.

  int dx = abs((int32_t)x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs((int32_t)y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2; /* error value e_xy */

  for (;;) { /* loop */
    draw_pixel(x0, y0, color);
    if (x0 == x1 && y0 == y1)
      break;
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    } /* e_xy+e_x > 0 */
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    } /* e_xy+e_y < 0 */
  }
}

void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color color) {
  draw_line(x, y, x + w, y, color);          // top edge
  draw_line(x, y + h, x + w, y + h, color);  // bottom edge
  draw_line(x, y, x, y + h, color);          // left edge
  draw_line(x + w, y, x + w, y + h, color);  // right edge
}

void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color color) {
  for (uint32_t i = x; i < x + w; ++i) {
    for (uint32_t j = y; j < y + h; ++j) {
      draw_pixel(i, j, color);
    }
  }
}

/** Rounds @x to the next multiple of @a multiple. */
[[nodiscard]] static uint32_t round_up(uint32_t x, uint32_t multiple) {
  return ((x + multiple - 1) / multiple) * multiple;
}

uint32_t draw_text(const char* text, Color color, uint32_t x, uint32_t y) {
  return draw_text(PKFFile(fonts_myfont_pkf), text, color, x, y);
}

uint32_t draw_text(PKFFile font, const char* text, Color color, uint32_t x, uint32_t y) {
  uint32_t current_x = x;
  uint32_t current_y = y;

  const uint32_t char_width = font.get_char_width();
  const uint32_t char_height = font.get_char_height();
  const uint32_t advance = font.get_advance();
  const uint32_t line_height = font.get_line_height();

  auto& fb = FrameBuffer::get();
  const uint32_t fb_width = fb.get_width();
  const uint32_t fb_height = fb.get_width();

  const uint8_t red = (color.argb >> 16) & 0xff;
  const uint8_t green = (color.argb >> 8) & 0xff;
  const uint8_t blue = color.argb & 0xff;

  for (const char* it = text; *it != '\0'; ++it) {
    const char ch = *it;

    if (ch >= PKFFile::FIRST_CHARACTER && ch <= PKFFile::LAST_CHARACTER) {
      if (current_x >= fb_width)
        continue;

      const uint8_t* glyph = font.get_glyph(ch);
      for (uint32_t i = 0; i < char_width; ++i) {
        for (uint32_t j = 0; j < char_height; ++j) {
          const uint8_t alpha = glyph[i + j * char_width];
          draw_pixel(current_x + i, current_y + j, make_color(red, green, blue, alpha));
        }
      }

      current_x += advance;
    } else {
      switch (ch) {
        case ' ':
          current_x += advance;
          break;
        case '\t':
          // We suppose that the tab size is 4.
          current_x = round_up(current_x, 4 * advance);
          break;
        case '\b':
          current_x -= advance;
          break;
        case '\r':
        case '\n':  // we handle CR and LF the same
          current_x = x;
          current_y += line_height;
          break;
        default:  // ignore the unknown character
          break;
      }
    }

    if (current_y >= fb_height)
      return current_x;
  }

  return current_x;
}
}  // namespace graphics
