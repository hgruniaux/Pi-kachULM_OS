#include "graphics/pkfont.hpp"

#include <libk/string.hpp>

uint32_t PKFont::get_horizontal_advance(const char* text, uint32_t length) const {
  if (length == UINT32_MAX) {
    length = libk::strlen(text);
  }

  return get_horizontal_advance() * length;
}

uint32_t PKFont::get_width(const char* text, uint32_t length) const {
  if (length == UINT32_MAX) {
    length = libk::strlen(text);
  }

  return get_char_width() * length;
}

const uint8_t* PKFont::get_glyph(char code) const {
  if (code < FIRST_CHARACTER || code > LAST_CHARACTER)
    return nullptr;  // the font does not contain this glyph

  const uint32_t char_width = get_char_width();
  const uint32_t char_height = get_char_height();

  const uint8_t index = (uint8_t)(code)-FIRST_CHARACTER;
  const uint8_t* offset = m_buffer + sizeof(PKFHeader) + (size_t)((char_width * char_height) * index);
  return offset;
}
