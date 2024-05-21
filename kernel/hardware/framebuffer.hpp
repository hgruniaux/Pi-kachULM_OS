#pragma once

#include <cstdint>

/**
 * There can only be one framebuffer at any time that can be accessed using get().
 * Before any use of the framebuffer, it must be initialized using init().
 *
 * You can modify read from and write to the framebuffer using the following functions:
 * - get_pixel(): gets the color of a specific pixel
 * - set_pixel(): sets the color of a specific pixel
 * - fill_rect(): fills a given rectangle with a given color
 * - clear(): clears all the framebuffer with a given color
 *
 * Once you have rendered a full frame, you should call:
 * - present(): presents the current framebuffer to the screen
 *
 * Example:
 * ```c++
 * FrameBuffer& fb = FrameBuffer::get();
 * const bool success = fb.init(1920, 1080);
 * if (!success) {
 *   // ERROR
 * }
 *
 * while (true) {
 *   fb.clear();
 *
 *   // Render your frame...
 *   fb.fill_rect(50, 50, 50, 50, 0xFF00FF00);
 *   fb.set_pixel(128, 666, 0xFF112233);
 *
 *   fb.present();
 * }
 * ```
 */
class FrameBuffer {
 public:
  /** @brief Returns the framebuffer instance. It should be initialized first. */
  static FrameBuffer& get();

  /** Checks if the framebuffer is correctly initialized (init() called and returned true). */
  [[nodiscard]] bool is_initialized() const { return m_initialized; }
  /** @brief Initializes a framebuffer of the given size. */
  bool init(uint32_t width, uint32_t height);

  /** @brief Converts a color to 0xAARRGGBB format suitable to be directly written to the buffer. */
  [[gnu::always_inline, nodiscard]] constexpr static uint32_t from_rgb(uint8_t r,
                                                                       uint8_t g,
                                                                       uint8_t b,
                                                                       uint8_t a = 0xff) {
    return (a << 24) | (r << 16) | (g << 8) | b;
  }

  /** @brief Clears the framebuffer with the given @a color in 0xAARRGGBB format. */
  void clear(uint32_t color = 0x0);
  void clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) { clear(from_rgb(r, g, b, a)); }

  /** @brief Gets the pixel in 0xAARRGGBB format at (x, y). */
  [[nodiscard]] uint32_t get_pixel(uint32_t x, uint32_t y) const;
  /** @brief Sets the pixel at (x, y) to @a color in 0xAARRGGBB format. */
  void set_pixel(uint32_t x, uint32_t y, uint32_t color);
  [[gnu::always_inline]] void set_pixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) {
    set_pixel(x, y, from_rgb(r, g, b, a));
  }

  /** @brief Presents the current framebuffer to the screen.
   *
   * To be called after a frame was rendered.
   *
   * This effectively swaps the front and back buffer if double buffering
   * is enabled. Otherwise, this function does nothing. */
  void present();

  /** @brief Gets the internal framebuffer buffer. */
  [[nodiscard]] uint32_t* get_buffer() { return m_buffer; }
  [[nodiscard]] const uint32_t* get_buffer() const { return m_buffer; }

  /** @brief Gets the framebuffer width, in pixels. */
  [[nodiscard]] uint32_t get_width() const { return m_width; }
  /** @brief Gets the framebuffer height, in pixels. */
  [[nodiscard]] uint32_t get_height() const { return m_height; }
  /** @brief Gets the framebuffer pitch, in pixels. */
  [[nodiscard]] uint32_t get_pitch() const { return m_pitch; }
  /** @brief Gets the framebuffer size, in bytes. */
  [[nodiscard]] uint32_t get_byte_size() const { return m_buffer_size * sizeof(uint32_t); }

 private:
  // Private constructor so there can be only once instance of Framebuffer
  // at any time accessed using get().
  FrameBuffer() = default;

  /** @brief Sends a SET_VIRTUAL_OFFSET request to VideoCore. */
  bool set_virtual_offset(uint32_t x, uint32_t y);

  uint32_t* m_buffer = nullptr;
  uint32_t m_buffer_size = 0;  // in count of uint32_t, the size of either front or back buffer
  uint32_t m_width = 0;        // in pixels
  uint32_t m_height = 0;       // in pixels
  uint32_t m_pitch = 0;        // length of a row, in pixels (this may be greater than the frame width)
#ifdef CONFIG_USE_DOUBLE_BUFFERING
  // display screen when using double buffering
  bool m_is_screen0 = true;
#endif  // CONFIG_USE_DOUBLE_BUFFERING
  bool m_initialized = false;
};  // class FrameBuffer
