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

class PKFFile {
 public:
  /** @brief First ASCII character included in the font. */
  static constexpr uint8_t FIRST_CHARACTER = 0x21;
  /** @brief Last ASCII character included in the font. */
  static constexpr uint8_t LAST_CHARACTER = 0x7e;

  constexpr PKFFile(const uint8_t* buffer) : m_buffer(buffer) {}

  /** @brief Access the @c char_width field of the header. */
  [[nodiscard]] uint32_t get_char_width() const { return reinterpret_cast<const PKFHeader*>(m_buffer)->char_width; }
  /** @brief Access the @c  char_height field of the header. */
  [[nodiscard]] uint32_t get_char_height() const { return reinterpret_cast<const PKFHeader*>(m_buffer)->char_height; }
  /** @brief Access the @c advance field of the header. */
  [[nodiscard]] uint32_t get_advance() const { return reinterpret_cast<const PKFHeader*>(m_buffer)->advance; }
  /** @brief Access the @c line_height field of the header. */
  [[nodiscard]] uint32_t get_line_height() const { return reinterpret_cast<const PKFHeader*>(m_buffer)->line_height; }

  [[nodiscard]] const uint8_t* get_glyph(char code) const;

 private:
  const uint8_t* m_buffer;
};  // class PKFFile
