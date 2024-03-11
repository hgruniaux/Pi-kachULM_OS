#include "pkf.hpp"

#include <cstddef>

const uint8_t* PKFFile::get_glyph(char code) const {
  if (code < FIRST_CHARACTER || code > LAST_CHARACTER)
    return nullptr;  // the font does not contain this glyph

  const uint32_t char_width = get_char_width();
  const uint32_t char_height = get_char_height();

  const uint8_t index = (uint8_t)(code)-FIRST_CHARACTER;
  const uint8_t* offset = m_buffer + sizeof(PKFHeader) + (size_t)((char_width * char_height) * index);
  return offset;
}
