# The PKF Format

PKF stands for **P**i-kachULM **K**ernel **F**ont. It is the internal format
to represent bitmap fonts used inside the kernel.

## Specification

The file format is quite simple. It starts by a header specified in the below section, and then
an array of glyph bitmap that is stored directly after the header.

### The header

```c++
struct PKFHeader {
    // Width, in pixels, of a character.
    uint32_t char_width;
    // Height, in pixels, of a character.
    uint32_t char_height;
    // Distance, in pixels, between two consecutive characters.
    uint32_t advance;
    // Distance, in pixels, baseline-to-baseline.
    uint32_t line_height;
};
```

All fields are in little-endian.

### The glyph array

The font only store the representation of displayable ASCII characters. That is, by convention,
all characters from `0x21` to `0x7e` both included. Each glyph have the same size (monospace font),
therefore accessing the buffer of one glyph is trivial:

```c++
const size_t index = REQUESTED_ASCII_CODE - 0x21;
const size_t glyph_buffer_size = sizeof(uint8_t) * char_width * char_height;
const uint8_t* glyph_buffer = pkf_buffer + sizeof(PKFHeader) + glyph_buffer_size * index;
```

where `REQUESTED_ASCII_CODE` is a `char` storing an ASCII code, and `pkf_buffer` is the start of the PKF file.

A glyph buffer is just an alpha map. A matrix of size `char_width`x`char_height` of `uint8_t`s representing an alpha
component (0 = transparent, 255 = fully opaque). The matrix is in row-major order. So, accessing an alpha of the glyph
at (x, y) is done with:

```c++
const uint8_t alpha = glyph_buffer[x + char_height * y];
```
