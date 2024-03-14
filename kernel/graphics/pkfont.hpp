#pragma once

#include <cstdint>

struct alignas(16) PKFHeader {
  /** @brief Width of a character, in pixels. */
  uint32_t char_width;
  /** @brief Height of a character, in pixels. */
  uint32_t char_height;
  /** @brief The default character spacing (distance between two consecutive characters), in pixels. */
  uint32_t advance;
  /** @brief The default line spacing (baseline-to-baseline distance), in pixels. */
  uint32_t line_height;
};  // struct PKFHeader

class PKFont {
 public:
  /** @brief First ASCII character included in the font. */
  static constexpr uint8_t FIRST_CHARACTER = 0x21;
  /** @brief Last ASCII character included in the font. */
  static constexpr uint8_t LAST_CHARACTER = 0x7e;

  /** @brief Creates a PKFont from the given @a buffer.
   *
   * The length of the @a buffer is implicit. Moreover, this function expect
   * the @a buffer is well-defined (the function is not safe). */
  constexpr PKFont(const uint8_t* buffer) : m_buffer(buffer) {}

  /** @brief Gets the width of a character in pixels.
   *
   * Access the @c char_width field of the header. */
  [[nodiscard]] uint32_t get_char_width() const { return reinterpret_cast<const PKFHeader*>(m_buffer)->char_width; }
  /** @brief Gets the height of a character in pixels.
   *
   * Access the @c char_height field of the header. */
  [[nodiscard]] uint32_t get_char_height() const { return reinterpret_cast<const PKFHeader*>(m_buffer)->char_height; }
  /** @brief Access the @c line_height field of the header. */
  [[nodiscard]] uint32_t get_line_height() const { return reinterpret_cast<const PKFHeader*>(m_buffer)->line_height; }

  /** @brief Returns the horizontal advance of a character in pixels.
   *
   * This is a distance appropriate for drawing a subsequent character after @a ch.
   *
   * Access the @c advance field of the header. */
  [[nodiscard]] uint32_t get_horizontal_advance() const {
    return reinterpret_cast<const PKFHeader*>(m_buffer)->advance;
  }

  /** @brief Returns the horizontal advance of character @a ch in pixels.
   *
   * This is a distance appropriate for drawing a subsequent character after @a ch.
   *
   * For monospace fonts (like PKF by default), the advance is the same for all characters. */
  [[nodiscard]] uint32_t get_horizontal_advance(char ch [[maybe_unused]]) const { return get_horizontal_advance(); }
  /** @brief Returns the horizontal advance of @a text in pixels.
   *
   * This is a distance appropriate for drawing a subsequent character after @a text.
   *
   * If @a length is UINT32_MAX, the @a text is assumed to be NUL-terminated. */
  [[nodiscard]] uint32_t get_horizontal_advance(const char* text, uint32_t length = UINT32_MAX) const;

  /** @brief Returns the width of @a text in pixels.
   *
   * If @a length is UINT32_MAX, the @a text is assumed to be NUL-terminated. */
  [[nodiscard]] uint32_t get_width(const char* text, uint32_t length = UINT32_MAX) const;

  /** @brief Returns the glyph alpha map corresponding of ASCII character @a code.
   *
   * A NULL pointer is returned if the glyph doesnt exist or is not supported by the font.
   *
   * The returned alpha map is a 2D matrix of uint8_t (alpha component, 0 = transparent, 255 = opaque),
   * in row-major format. To access the alpha at (x,y), you query it with `buffer[x + char_width * y]`.
   */
  [[nodiscard]] const uint8_t* get_glyph(char code) const;

 private:
  const uint8_t* m_buffer;
};  // class PKFont
