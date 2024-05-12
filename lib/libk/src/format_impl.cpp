#include "format_impl.hpp"

#include <libk/assert.hpp>
#include <libk/string.hpp>
#include <libk/utils.hpp>

#include "ryu/ryu.h"

namespace libk::detail {
static constexpr const char LOWER_DIGIT_ALPHABET[] = "0123456789abcdefghijklmnopqrstuvwxyz";
static constexpr const char UPPER_DIGIT_ALPHABET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static char* uint_to_string(uint64_t value,
                            char* buffer,
                            uint32_t radix,
                            bool lowercase = true,
                            size_t width = SIZE_MAX) {
  size_t i = 0;

  const char* alphabet = lowercase ? LOWER_DIGIT_ALPHABET : UPPER_DIGIT_ALPHABET;

  // Handle the case when the number is 0 separately
  if (value == 0) {
    buffer[i++] = '0';
  }

  // Extract digits in reverse order
  while (value > 0) {
    const auto digit = value % radix;
    buffer[i++] = alphabet[digit];
    value /= radix;
  }

  // Padding
  while ((width < SIZE_MAX) && i < width)
    buffer[i++] = '0';

  // Reverse the string to get the correct order
  size_t start = 0, end = i - 1;
  while (start < end) {
    const char temp = buffer[start];
    buffer[start] = buffer[end];
    buffer[end] = temp;
    start++;
    end--;
  }

  return buffer + i;
}

enum class SignFormat : uint8_t {
  /** Indicates that a sign should be used for both nonnegative as well as negative numbers.  */
  ALWAYS,
  /** Indicates that a sign should be used only for negative numbers (this is the default behavior). */
  ONLY_MINUS,
  /** Indicates that a leading space should be used on nonnegative numbers, and a minus sign on negative numbers. */
  SPACE
};  // enum class SignFormat

static const char* parse_sign_format(const char* spec, SignFormat& sign_format) {
  switch (*spec) {
    case '+':
      sign_format = SignFormat::ALWAYS;
      return spec + 1;
    case '-':
      sign_format = SignFormat::ONLY_MINUS;
      return spec + 1;
    case ' ':
      sign_format = SignFormat::SPACE;
      return spec + 1;
    default:
      sign_format = SignFormat::ONLY_MINUS;
      return spec;
  }
}

struct IntegerSpec {
  size_t width = SIZE_MAX;
  int radix = 10;
  SignFormat sign_format = SignFormat::ONLY_MINUS;
  bool add_prefix = false;
  bool lowercase = true;
};  // struct IntegerSpec

static char* output_sign(char* out, bool positive, SignFormat sign_format) {
  // Add sign if required.
  switch (sign_format) {
    case SignFormat::ALWAYS:
      if (!positive)
        *out++ = '-';
      else
        *out++ = '+';
      break;
    case SignFormat::ONLY_MINUS:
      if (!positive)
        *out++ = '-';
      break;
    case SignFormat::SPACE:
      if (!positive)
        *out++ = '-';
      else
        *out++ = ' ';
      break;
  }

  return out;
}

static char* output_uint(char* out, uintmax_t value, const IntegerSpec& spec) {
  // Add prefix if requested.
  if (spec.add_prefix) {
    const char* prefix;
    if (spec.radix == 2)
      prefix = spec.lowercase ? "0b" : "0B";
    else if (spec.radix == 16)
      prefix = spec.lowercase ? "0x" : "0X";
    else
      prefix = nullptr;

    if (prefix != nullptr) {
      strcpy(out, prefix);
      out += 2;
    }
  }

  KASSERT(spec.radix >= 2 && spec.radix <= 36);
  return uint_to_string(value, out, spec.radix, spec.lowercase, spec.width);
}

static IntegerSpec parse_integer_spec(const char* spec) {
  IntegerSpec parsed_spec = {};
  if (spec == nullptr)
    return parsed_spec;

  spec = parse_sign_format(spec, parsed_spec.sign_format);

  // Parse alternate form
  if (*spec == '#') {
    parsed_spec.add_prefix = true;
    ++spec;
  }

  // Parse type
  switch (*spec++) {
    case 'b':
      parsed_spec.radix = 2;
      parsed_spec.lowercase = true;
      break;
    case 'B':
      parsed_spec.radix = 2;
      parsed_spec.lowercase = false;
      break;
    case 'd':
      parsed_spec.radix = 10;
      break;
    case 'o':
      parsed_spec.radix = 8;
      break;
    case 'x':
      parsed_spec.radix = 16;
      parsed_spec.lowercase = true;
      break;
    case 'X':
      parsed_spec.radix = 16;
      parsed_spec.lowercase = false;
      break;
    default:
      spec--;
  }

  KASSERT(*spec == '}' && "unknown integer format specifier");
  return parsed_spec;
}

static char* format_int_to(char* out, intmax_t value, const char* spec) {
  const auto parsed_spec = parse_integer_spec(spec);
  out = output_sign(out, value > 0, parsed_spec.sign_format);
  return output_uint(out, (value >= 0) ? value : -value, parsed_spec);
}

static char* format_uint_to(char* out, uintmax_t value, const char* spec) {
  const auto parsed_spec = parse_integer_spec(spec);
  return output_uint(out, value, parsed_spec);
}

static char* format_pointer_to(char* out, const void* value, const char* spec) {
  KUNUSED(spec);
  IntegerSpec parsed_spec = {};
  parsed_spec.radix = 16;
  parsed_spec.add_prefix = true;
  parsed_spec.width = sizeof(void*) * 2;  // 2 hexadecimal digits per byte
  const auto int_value = (uintptr_t)value;
  return output_uint(out, int_value, parsed_spec);
}

static char* format_bool_to(char* out, bool value, const char* spec) {
  if (spec != nullptr && *spec != '}') {
    return format_int_to(out, (intmax_t)value, spec);
  }

  if (value) {
    strcpy(out, "true");
    return out + 4;
  }

  strcpy(out, "false");
  return out + 5;
}
/* // FIXME : Activate when floating point is activated

enum class FloatFormat : uint8_t { GENERAL, FIXED, EXP };

struct FloatSpec {
  FloatFormat format = FloatFormat::GENERAL;
  SignFormat sign_format = SignFormat::ONLY_MINUS;
  uint32_t precision = 3;
};  // struct FloatSpec

static FloatSpec parse_float_spec(const char* spec) {
  FloatSpec parsed_spec = {};
  if (spec == nullptr)
    return parsed_spec;

  spec = parse_sign_format(spec, parsed_spec.sign_format);

  if (*spec == '#') {
    // Ignore the special form. In the fmt library, the alternate form causes
    // the result of the conversion to always contain a decimal-point character,
    // even if no digits follow it.
    ++spec;
  }

  // Parse precision specifier
  if (*spec == '.') {
    ++spec;

    uint32_t precision = 0;
    while (*spec >= '0' && *spec <= '9') {
      precision *= 10;
      precision += *spec - '0';
      spec++;
    }

    if (precision == 0)
      precision = 1;

    parsed_spec.precision = precision;
  }

  switch (*spec++) {
    case 'f':
    case 'F':
      parsed_spec.format = FloatFormat::FIXED;
      break;
    case 'e':
    case 'E':
      parsed_spec.format = FloatFormat::EXP;
      break;
    case 'g':
    case 'G':
      parsed_spec.format = FloatFormat::GENERAL;
      break;
    default:
      --spec;
      break;
  }

  KASSERT(*spec == '}' && "unknown floating-point format specifier");
  return parsed_spec;
}

static char* format_float_to(char* out, float value, const char* spec) {
  const FloatSpec parsed_spec = parse_float_spec(spec);
  out = output_sign(out, value > 0.f, parsed_spec.sign_format);
  value = (value > 0.f) ? value : -value;
  switch (parsed_spec.format) {
    case FloatFormat::GENERAL:
      return out + f2s_buffered_n(value, out);
    case FloatFormat::FIXED:
      return out + d2fixed_buffered_n(value, parsed_spec.precision, out);
    case FloatFormat::EXP:
      return out + d2exp_buffered_n(value, parsed_spec.precision, out);
  }

  KASSERT(false && "unsupported floating point format type");
}

static char* format_double_to(char* out, double value, const char* spec) {
  const FloatSpec parsed_spec = parse_float_spec(spec);
  out = output_sign(out, value > 0., parsed_spec.sign_format);
  value = (value > 0.) ? value : -value;
  switch (parsed_spec.format) {
    case FloatFormat::GENERAL:
      return out + d2s_buffered_n(value, out);
    case FloatFormat::FIXED:
      return out + d2fixed_buffered_n(value, parsed_spec.precision, out);
    case FloatFormat::EXP:
      return out + d2exp_buffered_n(value, parsed_spec.precision, out);
  }

  KASSERT(false && "unsupported floating point format type");
}
*/
struct StringSpec {
  bool debug = false;
};  // struct StringSpec

static StringSpec parse_string_spec(bool is_char, const char* spec) {
  StringSpec parsed_spec = {};
  if (spec == nullptr)
    return parsed_spec;

  for (const char* spec_it = spec; *spec_it != '}'; ++spec_it) {
    if (*spec_it == '$') {
      parsed_spec.debug = true;
      continue;
    }

    if (is_char && *spec_it == 'c') {
      parsed_spec.debug = false;
      continue;
    }

    if (!is_char && *spec_it == 's') {
      parsed_spec.debug = false;
      continue;
    }

    KASSERT(false && "unknown string/char format specifier");
  }

  return parsed_spec;
}

static char* output_debug_char(char* out, char value) {
  switch (value) {
    case '\'':
      *out++ = '\\';
      *out++ = '\'';
      break;
    case '\t':
      *out++ = '\\';
      *out++ = 't';
      break;
    case '\n':
      *out++ = '\\';
      *out++ = 'n';
      break;
    case '\r':
      *out++ = '\\';
      *out++ = 'r';
      break;
    default:
      if ((value < 0x20 || value > 0x7e)) {
        *out++ = '\\';
        *out++ = 'x';
        *out++ = LOWER_DIGIT_ALPHABET[value / 16];
        *out++ = LOWER_DIGIT_ALPHABET[value % 16];
      } else {
        *out++ = value;
      }
  }

  return out;
}

static char* format_char_to(char* out, char value, const char* spec) {
  const auto parsed_spec = parse_string_spec(true, spec);
  if (parsed_spec.debug) {
    *out++ = '\'';
    out = output_debug_char(out, value);
    *out++ = '\'';
    return out;
  } else {
    *out = value;
    return out + 1;
  }
}

static char* format_string_to(char* out, const char* value, size_t length, const char* spec) {
  const auto parsed_spec = parse_string_spec(false, spec);
  if (parsed_spec.debug) {
    *out++ = '"';
    for (size_t i = 0; i < length; ++i) {
      out = output_debug_char(out, value[i]);
    }
    *out++ = '"';
    return out;
  }

  memcpy(out, value, sizeof(char) * length);
  return out + length;
}

char* format_argument_to(char* out, const Argument& argument, const char* spec) {
  switch (argument.type) {
    case Argument::Type::BOOL:
      return format_bool_to(out, argument.data.bool_value, spec);
    case Argument::Type::CHAR:
      return format_char_to(out, argument.data.char_value, spec);
    case Argument::Type::INTMAX:
      return format_int_to(out, argument.data.intmax_value, spec);
    case Argument::Type::UINTMAX:
      return format_uint_to(out, argument.data.uintmax_value, spec);
    /* FIXME : Activate when floating point is activated
    case Argument::Type::FLOAT:
      return format_float_to(out, argument.data.float_value, spec);
    case Argument::Type::DOUBLE:
      return format_double_to(out, argument.data.double_value, spec);
      */
    case Argument::Type::POINTER:
      return format_pointer_to(out, argument.data.pointer_value, spec);
    case Argument::Type::STRING:
      return format_string_to(out, argument.data.string_value.value, argument.data.string_value.length, spec);
  }

  KASSERT(false && "argument kind not implemented");
  return nullptr;
}
}  // namespace libk::detail
