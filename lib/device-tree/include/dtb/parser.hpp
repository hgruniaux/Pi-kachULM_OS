#pragma once

#include <libk/utils.hpp>

class DeviceTreeParser {
 public:
  DeviceTreeParser() = default;

  [[nodiscard]] static bool from_memory(uintptr_t dts, DeviceTreeParser* dt_parser);
  [[nodiscard]] bool is_initialized() const { return _dts != 0; }

  [[nodiscard]] inline uint32_t get_struct_offset() const { return _struct_ofs; }
  [[nodiscard]] inline uint32_t get_string_offset() const { return _string_ofs; }
  [[nodiscard]] inline uint32_t get_reserved_memory_offset() const { return _res_mem_ofs; }
  [[nodiscard]] inline uint32_t get_version() const { return _version; }

  [[nodiscard]] inline uint32_t get_uint32(size_t byte_offset) const {
    // Device Tree Integers are Big Endian
    return libk::from_be(libk::read32(_dts + byte_offset));
  }

  [[nodiscard]] inline uint64_t get_uint64(size_t byte_offset) const {
    // Device Tree Integers are Big Endian
    return libk::from_be(libk::read64(_dts + byte_offset));
  }

  [[nodiscard]] inline const char* get_string(size_t byte_offset) const { return (const char*)(_dts + byte_offset); }

  [[nodiscard]] size_t skip_property(size_t offset) const;
  [[nodiscard]] size_t skip_node(size_t offset) const;

  bool operator==(const DeviceTreeParser& b) const { return _dts == b._dts; }

 private:
  uintptr_t _dts = 0;
  uint32_t _struct_ofs = 0;
  uint32_t _string_ofs = 0;
  uint32_t _res_mem_ofs = 0;
  uint32_t _version = 0;
};
