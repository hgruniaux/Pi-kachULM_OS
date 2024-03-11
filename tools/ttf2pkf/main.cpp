#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#define ERROR "\x1b[1;31merror:\xb[0m "

constexpr uint32_t FIRST_CHARACTER = 0x21;
constexpr uint32_t LAST_CHARACTER = 0x7e;

struct alignas(16) PKFHeader {
  uint32_t char_width;
  uint32_t char_height;
  uint32_t advance;
  uint32_t line_height;
};  // struct PKFHeader

bool handle_ft_error(FT_Error error, const char* context) {
  fprintf(stderr, ERROR "%s: %s\n", context, FT_Error_String(error));
  return false;
}

static bool write_to_file(const char* output_path, const uint8_t* buffer, size_t buffer_size) {
  FILE* output_file = fopen(output_path, "wb");
  if (output_file == nullptr) {
    fprintf(stderr, ERROR "failed to save into file '%s'\n", output_path);
    return false;
  }

  fwrite(buffer, sizeof(uint8_t), buffer_size, output_file);
  fclose(output_file);
  return true;
}

static uint8_t* render_ascii_code(FT_Face face, char ch, uint8_t* output_buffer) {
  const FT_UInt char_width = face->size->metrics.x_ppem;
  const FT_UInt char_height = (face->size->metrics.ascender - face->size->metrics.descender) / 64;
  const FT_UInt ascender = face->size->metrics.ascender / 64;

  const FT_UInt glyph_index = FT_Get_Char_Index(face, (uint32_t)ch);
  FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
  if (error) {
    handle_ft_error(error, "failed to load or render glyph");
    return nullptr;
  }

#if 0
    std::cout << "Glyph " << (char)(i) << "\n";
    std::cout << face->glyph->advance.x / 64 << " " << face->glyph->advance.x / 64 << "\n";
    std::cout << face->glyph->bitmap_left << " " << face->glyph->bitmap_top << "\n";
    std::cout << face->glyph->metrics.width / 64 << " " << face->glyph->metrics.height / 64 << "\n";
#endif

  FT_Bitmap& bitmap = face->glyph->bitmap;
  memset(output_buffer, 0, sizeof(uint8_t) * char_width * char_height);

  for (uint32_t y = 0; y < bitmap.rows; ++y) {
    for (uint32_t x = 0; x < bitmap.width; ++x) {
      uint8_t alpha = bitmap.buffer[x + y * bitmap.width];
      output_buffer[(face->glyph->bitmap_left + x) + char_width * (ascender - face->glyph->bitmap_top + y)] = alpha;
    }
  }

  output_buffer += char_width * char_height;
  return output_buffer;
}

bool convert(const char* input_path, const char* output_path, uint32_t requested_size_in_pixels = 10) {
  FT_Library library;
  FT_Error error = FT_Init_FreeType(&library);
  if (error)
    return handle_ft_error(error, "failed to initialize freetype");

  FT_Face face;
  error = FT_New_Face(library, input_path, 0, &face);
  if (error)
    return handle_ft_error(error, "failed to open font");

  if (!FT_IS_FIXED_WIDTH(face)) {
    fprintf(stderr, ERROR "the PKF format only support monospace fonts, but the provided one is not monospace");
    return false;
  }
  error = FT_Set_Pixel_Sizes(face, 0, requested_size_in_pixels);
  if (error)
    return handle_ft_error(error, "failed to set font size");

  uint32_t real_char_height = (face->size->metrics.ascender - face->size->metrics.descender) / 64;

  const size_t char_count = LAST_CHARACTER - FIRST_CHARACTER + 1;
  const size_t pkf_buffer_size = sizeof(PKFHeader) + (requested_size_in_pixels * real_char_height) * char_count;
  uint8_t* pkf_buffer = new uint8_t[pkf_buffer_size];

  // Fill in the header:
  PKFHeader* pkf_header = reinterpret_cast<PKFHeader*>(pkf_buffer);
  pkf_header->char_width = requested_size_in_pixels;
  pkf_header->char_height = real_char_height;
  pkf_header->line_height = face->size->metrics.height / 64;

  // We need to access M to retrieve its advance value. See the comment below.
  const FT_UInt glyph_index = FT_Get_Char_Index(face, (uint32_t)'M');
  error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
  if (error) {
    handle_ft_error(error, "failed to load the 'M' glyph");
    return false;
  }

  // We cannot use face->size->metrics.max_advance because some monospace fonts (like Fira Code)
  // have glyphs with a bigger advance (like ligatures). So instead, we retrieve the advance of
  // a basic ASCII letter like M.
  pkf_header->advance = face->glyph->advance.x / 64;

#if 0
  printf("Header:\n");
  printf("  - char_width: %u\n", pkf_header->char_width);
  printf("  - char_height: %u\n", pkf_header->char_height);
  printf("  - advance: %u\n", pkf_header->advance);
  printf("  - line_height: %u\n", pkf_header->line_height);
#endif

  // Render each glyphs:
  uint8_t* glyph_buffer = pkf_buffer + sizeof(PKFHeader);
  for (uint32_t i = FIRST_CHARACTER; i <= LAST_CHARACTER; ++i) {
    glyph_buffer = render_ascii_code(face, i, glyph_buffer);
    if (glyph_buffer == nullptr)
      return false;
  }

  if (!write_to_file(output_path, pkf_buffer, pkf_buffer_size))
    return false;

  FT_Done_FreeType(library);
  return true;
}

int main(int argc, char* argv[]) {
  bool convert_to_cxx = false;
  const char* output_file = nullptr;
  const char* input_file = nullptr;
  uint32_t font_size = 12;
  bool stop_parsing_options = false;
  for (int i = 1; i < argc; ++i) {
    if (!stop_parsing_options && strcmp(argv[i], "-c++") == 0) {
      convert_to_cxx = true;
    } else if (!stop_parsing_options && strcmp(argv[i], "-o") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, ERROR "missing argument after the option -o\n");
        return EXIT_FAILURE;
      }

      output_file = argv[i + 1];
      ++i;
    } else if (!stop_parsing_options && strcmp(argv[i], "-s") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, ERROR "missing argument after the option -s\n");
        return EXIT_FAILURE;
      }

      font_size = strtol(argv[i + 1], nullptr, 10);
      ++i;
    } else if (!stop_parsing_options && strcmp(argv[i], "--") == 0) {
      stop_parsing_options = true;
    } else {
      if (input_file != nullptr) {
        fprintf(stderr, ERROR "multiple input files provided\n");
        return EXIT_FAILURE;
      }

      input_file = argv[i];
    }
  }

  if (!convert(input_file, output_file, font_size))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
