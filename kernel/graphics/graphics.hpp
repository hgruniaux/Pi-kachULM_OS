#pragma once

#include <cstdint>

#include "pkf.hpp"

namespace graphics {
struct Color {
  uint32_t argb;
};  // struct Color

/** @brief Converts a color to 0xAARRGGBB format suitable to be directly written to the buffer. */
[[nodiscard]] inline Color make_color(uint32_t r, uint32_t g, uint32_t b, uint32_t a = 0xff) {
  return {(a << 24) | (r << 16) | (g << 8) | b};
}

/** Clears the global framebuffer with the given @a color. */
void clear(Color color);

/**
 * Draws a pixel at (@a x, @a y) of the given @a color in the global framebuffer.
 *
 * This function differs in many points from FrameBuffer::set_pixel():
 * - It does bounds checking. So if you try to draw outside the framebuffer, the request is just ignored (clipping).
 *   Whereas, FrameBuffer::set_pixel() kernel panic in such cases.
 * - It does alpha blending. So if you draw with a color not fully opaque, the color is blended with the
 *   color already in the framebuffer. Likewise, if you draw with a color fully transparent, nothing is drawn.
 *   FrameBuffer::set_pixel() ignores the alpha component of your color.
 *
 * Because of this, this function is slower than FrameBuffer::set_pixel(). But, it still
 * recommended to use it instead of the FrameBuffer interface.
 */
void draw_pixel(uint32_t x, uint32_t y, Color color);

/**
 * Draws a line from (@a x0, @a y0) to (@a x1, y1) in the given @a color in the global framebuffer.
 *
 * All coordinates and sizes are given in pixels.
 */
void draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, Color color);
/**
 * Draws a rectangle at (@a x, @a y) of size (@a w, @a h) in the given @a color in the global framebuffer.
 *
 * Only the borders of the rectangle are drawn. To fill the inside, call fill_rect() instead.
 *
 * All coordinates and sizes are given in pixels.
 */
void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color color);
/**
 * Same as draw_rect() but also fills the inside of the rectangle.
 */
void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color color);

uint32_t draw_text(const char* text, Color color, uint32_t x, uint32_t y);
uint32_t draw_text(PKFFile font, const char* text, Color color, uint32_t x, uint32_t y);
}  // namespace graphics
