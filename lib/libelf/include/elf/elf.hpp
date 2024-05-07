#pragma once

#include <cstdint>

/*
 * The following library can be used to read simple ELF files for execution by a kernel.
 * The library makes no claim to read the entire ELF standard, nor to support the majority
 * of its features. In particular, this library assumes that only 64-bit ELF files using
 * the little endian format will be read.
 *
 * How to use the library:
 * ```c++
 * // Ideally, bytes should be aligned to alignof(uint64_t)
 * const uint8_t* bytes = read_file("my_file.elf");
 * const elf::Header* elf = (const elf::Header*)bytes;
 * if (elf::check_header(elf) != elf::Error::NONE) {
 *    // ERROR to parse the ELF file
 *    return;
 * }
 *
 * // Example for a Type::EXEC ELF file:
 *
 * // Load the segments into memory
 * for (uint64_t i = 0; i < elf->program_header_entry_count; ++i) {
 *    const elf::ProgramHeader* segment = elf::get_program_header(elf, i);
 *    KASSERT(segment != nullptr);
 *
 *    if (segment->is_load()) {
 *        // This is an example:
 *        void* mem = mmap(segment->virtual_addr, segment->mem_size, some flags);
 *        if (segment->file_size > 0) {
 *            memcpy(mem, (const uint8_t*)(elf) + segment->offset, segment->file_size);
 *            ...
 *        }
 *
 *        // Handle BSS sections...
 *        // And other things
 *    }
 * }
 *
 * // Execute the program
 * PC = elf->entry_addr;
 * ```
 */

namespace elf {
// ELF magic number.
constexpr uint8_t ELF_MAG0 = 0x7f;
constexpr uint8_t ELF_MAG1 = 0x45;  // 'E'
constexpr uint8_t ELF_MAG2 = 0x4c;  // 'L'
constexpr uint8_t ELF_MAG3 = 0x46;  // 'F'

enum class Class : uint8_t {
  /** Invalid ELF class. */
  NONE = 0,
  /** ELF 32-bits. */
  ELF32 = 1,
  /** ELF 64-bits. */
  ELF64 = 2
};  // enum class Class

/** The different byte orders that can use an ELF file. */
enum class Endianness {
  /** Invalid endianness. */
  NONE = 0,
  /** Little-endian. */
  LITTLE = 1,
  /** Big-endian. */
  BIG = 2
};  // enum class Endianness

/** The different ELF file types. */
enum class Type : uint16_t {
  /** Unknown ELF file type. */
  NONE = 0,
  /** Relocatable ELF file type. */
  REL = 1,
  /** An executable ELF file type. */
  EXEC = 2,
  /** A shared object ELF file type. */
  DYN = 3,
  /** A core dump ELF file type. */
  CORE = 4
};  // enum class Type

/** The different instruction sets that can target an ELF file. */
enum class Machine : uint16_t {
  /** Unknown machine type. */
  NONE = 0,
  /** X86 machine type. */
  X86 = 0x3,
  /** X86_64 machine type. */
  X86_64 = 0x3E,
  /** ARM machine type. */
  ARM = 0x28,
  /** Aarch64 machine type. */
  AARCH64 = 0xB7
};  // enum class Machine

/** The different versions of ELF. */
enum class Version : uint32_t {
  /** Invalid version. */
  NONE = 0,
  /** Current version. */
  CURRENT_VERSION = 1
};  // enum class Version

/**
 * ELF 64-bits header.
 */
struct Header {
  // Used for ELF file identification. Ignore it, it is
  // the responsibility of the ELF parser.
  uint8_t ident[16];
  /** Identifies object file type. */
  Type type;
  /** Specifies target instruction set architecture.
   * On the kernel we only support Machine::Aarch64. */
  Machine machine;
  Version version; // always 1
  /** The virtual address of the program's entry point.
   * May be 0 if there is no entry point. */
  uint64_t entry_addr;
  uint64_t program_header_offset;
  uint64_t section_header_offset;
  [[maybe_unused]] uint32_t flags;  // unused
  uint16_t header_size;
  uint16_t program_header_entry_size;
  uint16_t program_header_entry_count;
  uint16_t section_header_entry_size;
  uint16_t section_header_entry_count;
};  // struct Header

/**
 * The different program header types allowed by the ELF standard.
 */
enum class ProgramType {
  NULL_TYPE = 0,
  /** A loadable segment. */
  LOAD = 1,
  /** Dynamic linking information. */
  DYNAMIC = 2,
  /** Interpreter information. */
  INTERPRETER = 3,
  /** Auxiliary information. */
  NOTE = 4,
  /** This segment is reserved but has unspecified semantics. */
  SHLIB = 5,
  /** Segment containing program header table itself. */
  PROGRAM_HEADER = 6,
  /** Thread-Local Storage template. */
  TLS = 7
}; // enum class ProgramType

/** The allowed flags in ProgramHeader::flags. */
enum class ProgramFlag : uint32_t {
  /** The section is executable. */
  EXECUTABLE = 0x1,
  /** The section is writable. */
  WRITABLE = 0x2,
  /** The section is readable. */
  READABLE = 0x4
};  // enum class SectionFlag

/**
 * ELF 64-bits program header.
 */
struct ProgramHeader {
  /** Identifies the type of the segment. */
  ProgramType type;
  /** Segment-dependent flags. See ProgramFlag. */
  uint32_t flags;  // see ProgramFlag
  /** Offset of the segment in the file image. */
  uint64_t offset;
  /** Virtual address of the segment in memory. */
  uint64_t virtual_addr;
  uint64_t physical_addr;  // unused, must be 0
  /** Size in bytes of the segment in the file image. May be 0. */
  uint64_t file_size;
  /** Size in bytes of the segment in memory. May be 0. */
  uint64_t mem_size;
  /** 0 and 1 specify no alignment. Otherwise should be a positive,
   * integral power of 2, with @c virtual_address equating @c offset
   * modulus @c alignment. */
  uint64_t alignment;

  /** Check if this segment should have readable permission. */
  [[nodiscard]] bool is_readable() const { return (flags & (uint32_t)ProgramFlag::READABLE) != 0; }

  /** Check if this segment should have writable permission. */
  [[nodiscard]] bool is_writable() const { return (flags & (uint32_t)ProgramFlag::WRITABLE) != 0; }

  /** Check if this segment should have executable permission. */
  [[nodiscard]] bool is_executable() const { return (flags & (uint32_t)ProgramFlag::EXECUTABLE) != 0; }

  /** Check if this segment is of type loadable. */
  [[nodiscard]] bool is_load() const { return type == ProgramType::LOAD; }
};  // struct ProgramHeader

/**
 * The different section types allowed by the ELF standard.
 */
enum class SectionType : uint32_t {
  NULL_TYPE = 0,
  PROGRAM_BITS = 1,
  SYMBOL_TABLE = 2,
  STRING_TABLE = 3,
  RELA = 4,
  /**
   * This section holds a symbol hash table. An object
   * participating in dynamic linking must contain a
   * symbol hash table. An object file may have only
   * one hash table.
   */
  HASH = 5,
  /**
   * This section holds information for dynamic linking.
   * An object file may have only one dynamic section.
   */
  DYNAMIC = 6,
  /**
   * This section holds notes
   */
  NOTE = 7,
  /**
   * A section of this type occupies no space in the
   * file but otherwise resembles PROGRAM_BITS.
   * Although this section contains no bytes, the
   * SectionHeader::offset member contains the conceptual file
   * offset.
   */
  NO_BITS = 8,
  REL = 9,
  /**
   * This section is reserved but has unspecified semantics.
   */
  SHLIB = 10,
  /**
   * This section holds a minimal set of dynamic linking symbols.
   */
  DYNAMIC_SYMBOLS = 11,
};  // enum class SectionType

/**
 * ELF 64-bits section header.
 */
struct SectionHeader {
  uint32_t name_offset;
  /** Identifies the type of this header. */
  SectionType type;
  /** Identifies the attributes of the section. */
  uint64_t flags;
  /** Virtual address of the section in memory, for sections that are loaded. */
  uint64_t virtual_addr;
  /** Offset of the section in the file image. */
  uint64_t offset;
  /** Size in bytes of the section in the file image. May be 0. */
  uint64_t size;
  /** Contains the section index of an associated section.
   * Its use depends on the section type. */
  uint32_t link;
  /** Contains extra information about the section.
   * Its use depends on the section type. */
  uint32_t info;
  /** Contains the required alignment of the section.
   * This field must be a power of two. */
  uint64_t alignment;
  /** Contains the size, in bytes, of each entry, for sections that contain
   * fixed-size entries. Otherwise, this field contains zero. */
  uint64_t entry_size;
};  // struct SectionHeader

/**
 * The different errors that can be generated by the ELF parser.
 */
enum class Error {
  NONE = 0,
  UNKNOWN_ERROR,
  INVALID_MAG0,
  INVALID_MAG1,
  INVALID_MAG2,
  INVALID_MAG3,
  UNSUPPORTED_CLASS,
  UNSUPPORTED_ENDIANNESS,
  UNSUPPORTED_MACHINE,
  UNSUPPORTED_VERSION
};  // enum class Error

/**
 * Check if the given ELF header is correct and supported by the kernel
 * (that is if represents an Aarch64 program using ELF 64-bits little-endian).
 *
 * @param header The ELF file.
 * @return Error::NONE if the header is correct, an error otherwise.
 */
[[nodiscard]] Error check_header(const Header* header);

/**
 * Check if the given ELF file has at least one program header (segment).
 *
 * @param header The ELF file.
 * @return True if there is at least one segment (hdr->program_header_entry_count > 0).
 */
[[nodiscard]] inline bool has_program_headers(const Header* header) {
  return header != nullptr && header->program_header_entry_count > 0;
}

/**
 * Check if the given ELF file has at least one section header.
 *
 * @param header The ELF file.
 * @return True if there is at least one section (hdr->section_header_entry_count > 0).
 */
[[nodiscard]] inline bool has_section_headers(const Header* header) {
  return header != nullptr && header->section_header_entry_count > 0;
}

/**
 * Get the program header at the given index of the given ELF file.
 *
 * @param header The ELF file.
 * @param idx The program header index.
 * @return Nullptr if the header is invalid or if the index is out of bounds, the program header otherwise.
 */
[[nodiscard]] const ProgramHeader* get_program_header(const Header* header, uint64_t idx);
[[nodiscard]] const SectionHeader* get_section_header(const Header* header, uint64_t idx);
}  // namespace elf
