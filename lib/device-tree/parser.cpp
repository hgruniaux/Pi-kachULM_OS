#include "dtb/parser.hpp"

#include "libk/assert.hpp"
#include "libk/string.hpp"
#include "libk/utils.hpp"
#include "utils.hpp"

DeviceTreeParser DeviceTreeParser::from_memory(const void* dts) {
  if (libk::align(dts, alignof(uint64_t)) != dts) {
    return DeviceTreeParser(nullptr, 0, 0, 0, 0);
  }

  const auto* header = (const uint32_t*)dts;

  if (libk::from_be(header[0]) != DTB_MAGIC) {
    return DeviceTreeParser(nullptr,  0, 0, 0, 0);
  }

  const uint32_t off_struct = libk::from_be(header[2]);
  if (libk::align(off_struct, alignof(uint32_t)) != off_struct) {
    // Unaligned property section
    return DeviceTreeParser(nullptr, 0, 0, 0,  0);
  }

  const uint32_t off_strings = libk::from_be(header[3]);

  const uint32_t off_mem_reserved_map = libk::from_be(header[4]);
  if (libk::align(off_mem_reserved_map, alignof(uint64_t)) != off_mem_reserved_map) {
    // Unaligned reserved memory section
    return DeviceTreeParser(nullptr, 0, 0, 0, 0);
  }

  const uint32_t version = libk::from_be(header[5]);

  return DeviceTreeParser((const uint8_t*)dts, off_struct, off_strings, off_mem_reserved_map, version);
}

size_t DeviceTreeParser::skip_property(size_t offset) const {
  KASSERT(get_uint32(offset) == DTB_PROP);

  // Ignore PROP token
  offset += sizeof(uint32_t);

  // Get property size and skip it
  const size_t property_size = get_uint32(offset);
  offset += sizeof(uint32_t);

  // Skip property name
  offset += sizeof(uint32_t);

  // Skip property data
  offset = libk::align(offset + property_size, alignof(uint32_t));

  return offset;
}

size_t DeviceTreeParser::skip_node(size_t offset) const {
  KASSERT(get_uint32(offset) == DTB_BEGIN_NODE);

  // Ignore BEGIN_NODE token
  offset += sizeof(uint32_t);

  // And ignore the node's name
  const size_t name_size = libk::strlen(get_string(offset)) + 1;  // We count the \000 at the end
  offset = libk::align(offset + name_size, alignof(uint32_t));

  while (true) {
    const uint32_t token = get_uint32(offset);
    switch (token) {
      case DTB_BEGIN_NODE: {
        // Skip Child
        offset = skip_node(offset);
        break;
      }

      case DTB_NOP: {
        // Ignore NOP Token
        offset += sizeof(uint32_t);
        break;
      }

      case DTB_PROP: {
        // Skip Property
        offset = skip_property(offset);
        break;
      }

      case DTB_END_NODE: {
        // End of current node
        return offset + sizeof(uint32_t);
      }

      default: {
        KASSERT(false);
        // TODO : Fix this with a message
        // LOG_CRITICAL("Unrecognized token in Device Tree Blob at offset {} : {}.", offset, token);
      }
    }
  }
}
