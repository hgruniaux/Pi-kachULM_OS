#pragma once

#include <cstdint>

#include "pkf.hpp"

namespace graphics {
/** @brief Represents a ARGB color that can be used in the Kernel graphics API. */
struct Color {
  uint32_t argb;

  /** @brief Creates an uninitialized color. */
  constexpr Color() = default;
  /** @brief Creates a color given @a argb in 0xAARRGGBB format. */
  constexpr Color(uint32_t argb) : argb(argb) {}
  /** @brief Creates a color from the given red, green, blue and alpha components (in range [0, 255]. */
  constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) : argb((a << 24) | (r << 16) | (g << 8) | b) {}

  static constexpr uint32_t WHITE = 0xFFFFFFFF;
  static constexpr uint32_t BLACK = 0xFF000000;
  static constexpr uint32_t GRAY = 0xFF808080;
  static constexpr uint32_t RED = 0xFFFF0000;
  static constexpr uint32_t GREEN = 0xFF00FF00;
  static constexpr uint32_t BLUE = 0xFF0000FF;

  /** @brief Gets the alpha component, in the range [0, 255]. */
  [[nodiscard]] uint8_t get_alpha() const { return (argb >> 24) & 0xff; }
  /** @brief Sets the alpha component to @a a, in range [0, 255]. */
  void set_alpha(uint8_t a) { argb = (argb & 0x00ffffff) | (a << 24); }

  /** @brief Gets the red component, in the range [0, 255]. */
  [[nodiscard]] uint8_t get_red() const { return (argb >> 16) & 0xff; }
  /** @brief Sets the red component to @a r, in range [0, 255]. */
  void set_red(uint8_t r) { argb = (argb & 0xff00ffff) | (r << 16); }

  /** @brief Gets the green component, in the range [0, 255]. */
  [[nodiscard]] uint8_t get_green() const { return (argb >> 8) & 0xff; }
  /** @brief Sets the green component to @a g, in range [0, 255]. */
  void set_green(uint8_t g) { argb = (argb & 0xffff00ff) | (g << 8); }

  /** @brief Gets the blue component, in the range [0, 255]. */
  [[nodiscard]] uint8_t get_blue() const { return (argb) & 0xff; }
  /** @brief Sets the blue component to @a b, in range [0, 255]. */
  void set_blue(uint8_t b) { argb = (argb & 0xffffff00) | b; }

  /** @brief Returns a copy of this color with the given @a a alpha component (in range [0,255]). */
  [[nodiscard]] Color with_alpha(uint8_t a) const { return ((argb & 0xffffff) | (a << 24)); }
};  // struct Color

/** @brief Converts a color to 0xAARRGGBB format suitable to be directly written to the buffer. */
[[nodiscard]] inline Color make_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) {
  return {((uint32_t)a << 24) | (r << 16) | (g << 8) | b};
}

/**
 * @brief Painter provides an interface for drawing in a framebuffer.
 *
 * ## Drawing functions
 *
 * Drawing functions are available for most primitives. For example, draw_line(), draw_rect(), draw_pixel(), etc.
 *
 * These functions take integer coordinates that must be expressed in framebuffer space.
 * In addition, the color of the drawing can either be specified explicitly as a function parameter.
 * Or, implicitly, with the pen color. See get_pen() and set_pen() functions.
 *
 * ## Difference between FrameBuffer::set_pixel() and draw_pixel()
 *
 * Although the two functions may appear to do the same thing, they are radically different. Indeed:
 * - draw_pixel() supports clipping (implicit with framebuffer borders, or explicit with the set_clipping function). In
 *   other words, with set_pixel(), writing outside the framebuffer causes a kernel panic. Whereas with draw_pixel(),
 *   writing is simply ignored.
 * - draw_pixel() supports alpha blending. In other words, if the color is not totally opaque, then it will be blended
 *   with the background color. Similarly, if a color is completely transparent, it will not be written.
 *
 * @see Color
 */
class Painter {
 public:
  /** @brief Creates a painter that draws into the global framebuffer. See FrameBuffer class. */
  Painter();
  /** @brief Creates a painter that draws into the provided framebuffer. */
  Painter(uint32_t* buffer, uint32_t width, uint32_t height, uint32_t pitch);

  /** @brief Gets the current used color to draw. */
  [[nodiscard]] Color get_pen() const { return m_pen; }
  /** @brief Sets the color to use to draw.  */
  void set_pen(Color color) { m_pen = color; }
  void set_pen(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) { set_pen(make_color(r, g, b, a)); }

  /** @brief Clears the framebuffer with the given @a clear_color. */
  void clear(Color clear_color = make_color(0, 0, 0));

  // Pixel drawing functions:
  void draw_pixel(uint32_t x, uint32_t y);
  void draw_pixel(uint32_t x, uint32_t y, Color color);

  // Line drawing functions:
  void draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);
  void draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, Color color);

  // Rectangle drawing functions:
  void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
  void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color color);
  void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
  void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, Color color);

  // Text drawing functions:
  uint32_t draw_text(uint32_t x, uint32_t y, const char* text);
  uint32_t draw_text(uint32_t x, uint32_t y, const char* text, Color color);
  uint32_t draw_text(uint32_t x, uint32_t y, uint32_t w, const char* text);
  uint32_t draw_text(uint32_t x, uint32_t y, uint32_t w, const char* text, Color color);

  // Clipping functions:
  void revert_clipping();
  void set_clipping(uint32_t x_min, uint8_t y_min, uint32_t x_max, uint32_t y_max);

 private:
  /** @brief Implements the Painter constructor. */
  void create(uint32_t* buffer, uint32_t width, uint32_t height, uint32_t pitch);
  /** @brief Used internally by draw_text() to draw a glyph alpha map. */
  void draw_alpha_map(uint32_t x, uint32_t y, const uint8_t* alpha_map, uint32_t w, uint32_t h, Color color);

  struct BBox {
    uint32_t x_min;
    uint32_t y_min;
    uint32_t x_max;
    uint32_t y_max;
  };  // struct BBox

  uint32_t* m_buffer;  // framebuffer, in 0xAARRGGBB format
  uint32_t m_width;    // width of the framebuffer, in pixels
  uint32_t m_height;   // height of the framebuffer, in pixels
  uint32_t m_pitch;    // pitch of the framebuffer
  BBox m_clipping;
  Color m_pen = make_color(0xff, 0xff, 0xff);
};  // class Painter
}  // namespace graphics
