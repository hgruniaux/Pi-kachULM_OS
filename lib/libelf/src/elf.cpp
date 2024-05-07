#include "elf/elf.hpp"

namespace elf {
constexpr uint8_t EI_MAG0 = 0;
constexpr uint8_t EI_MAG1 = 1;
constexpr uint8_t EI_MAG2 = 2;
constexpr uint8_t EI_MAG3 = 3;
constexpr uint8_t EI_CLASS = 4;
constexpr uint8_t EI_DATA = 5;
constexpr uint8_t EI_VERSION = 6;

Error check_header(const Header* header) {
  if (header == nullptr)
    return Error::UNKNOWN_ERROR;

  // Check magic number
  if (header->ident[EI_MAG0] != ELF_MAG0)
    return Error::INVALID_MAG0;
  if (header->ident[EI_MAG1] != ELF_MAG1)
    return Error::INVALID_MAG1;
  if (header->ident[EI_MAG2] != ELF_MAG2)
    return Error::INVALID_MAG2;
  if (header->ident[EI_MAG3] != ELF_MAG3)
    return Error::INVALID_MAG3;

  // Check ELF class (we only support 64-bits)
  if (header->ident[EI_CLASS] != (uint8_t)Class::ELF64)
    return Error::UNSUPPORTED_CLASS;

    // Check ELF endianness
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  if (header->ident[EI_DATA] != (uint8_t)Endianness::LITTLE)
    return Error::UNSUPPORTED_ENDIANNESS;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  if (header->ident[EI_DATA] != (uint8_t)Endianness::BIG)
    return Error::UNSUPPORTED_ENDIANNESS;
#else
#error Unknown byte order.
#endif

  // Check if it is an Aarch64 ELF file
  if (header->machine != Machine::AARCH64)
    return Error::UNSUPPORTED_MACHINE;

  // Check ELF version
  if (header->ident[EI_VERSION] != (uint8_t)Version::CURRENT_VERSION)
    return Error::UNSUPPORTED_VERSION;

  return Error::NONE;
}

const ProgramHeader* get_program_header(const Header* header, uint64_t idx) {
  if (header == nullptr)
    return nullptr;

  if (idx >= header->program_header_entry_count)
    return nullptr;

  const ProgramHeader* program_headers = (const ProgramHeader*)((const uint8_t*)header + header->program_header_offset);
  return &program_headers[idx];
}

const SectionHeader* get_section_header(const Header* header, uint64_t idx) {
  if (header == nullptr)
    return nullptr;

  if (idx >= header->section_header_entry_count)
    return nullptr;

  const SectionHeader* section_headers = (const SectionHeader*)((const uint8_t*)header + header->section_header_offset);
  return &section_headers[idx];
}
}  // namespace elf
