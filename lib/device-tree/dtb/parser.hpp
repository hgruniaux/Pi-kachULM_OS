#pragma once

#include <cstddef>
#include <cstdint>

#include "libk/utils.hpp"

class DeviceTreeParser {
 public:
  [[nodiscard]] static DeviceTreeParser from_memory(const void* dts);
  [[nodiscard]] bool is_status_okay() const { return version != 0; }

  const uint32_t total_size;
  const uint32_t struct_offset;
  const uint32_t string_offset;
  const uint32_t reserved_memory_offset;
  const uint32_t version;

  [[nodiscard]] inline uint32_t get_uint32(size_t byte_offset) const {
    // Device Tree Integers are Big Endian
    return libk::from_be(*((const uint32_t*)(m_dts + byte_offset)));
  }

  [[nodiscard]] inline uint64_t get_uint64(size_t byte_offset) const {
    // Device Tree Integers are Big Endian
    return libk::from_be(*((const uint64_t*)(m_dts + byte_offset)));
  }

  [[nodiscard]] const char* get_string(size_t byte_offset) const { return (const char*)(m_dts + byte_offset); }

  [[nodiscard]] size_t skip_property(size_t offset) const;
  [[nodiscard]] size_t skip_node(size_t offset) const;

  bool operator==(const DeviceTreeParser& b) const { return m_dts == b.m_dts; }

 private:
  explicit DeviceTreeParser(const uint8_t* dts,
                            uint32_t total_size,
                            uint32_t off_struct,
                            uint32_t off_strings,
                            uint32_t off_mem_reserved_map,
                            uint32_t version)
      : total_size(total_size),
        struct_offset(off_struct),
        string_offset(off_strings),
        reserved_memory_offset(off_mem_reserved_map),
        version(version),
        m_dts(dts) {}

  const uint8_t* m_dts;
};
