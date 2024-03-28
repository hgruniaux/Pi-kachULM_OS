#include "dtb/parser.hpp"

#include "libk/assert.hpp"
#include "libk/string.hpp"
#include "utils.hpp"

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define be32toh(x) __builtin_bswap32(x)
#else
#define be32toh(x) (x)
#endif

DeviceTreeParser DeviceTreeParser::from_memory(const void* dts) {
  if (align_pointer(dts, alignof(uint64_t)) != dts) {
    return DeviceTreeParser(nullptr, 0, 0, 0, 0, 0);
  }

  const auto* header = (const uint32_t*)dts;

  if (be32toh(header[0]) != DTB_MAGIC) {
    return DeviceTreeParser(nullptr, 0, 0, 0, 0, 0);
  }

  const uint32_t total_size = be32toh(header[1]);
  const uint32_t off_struct = be32toh(header[2]);
  if (align_pointer(off_struct, alignof(uint32_t)) != off_struct) {
    // Unaligned property section
    return DeviceTreeParser(nullptr, 0, 0, 0, 0, 0);
  }

  const uint32_t off_strings = be32toh(header[3]);

  const uint32_t off_mem_reserved_map = be32toh(header[4]);
  if (align_pointer(off_mem_reserved_map, alignof(uint64_t)) != off_mem_reserved_map) {
    // Unaligned reserved memory section
    return DeviceTreeParser(nullptr, 0, 0, 0, 0, 0);
  }

  const uint32_t version = be32toh(header[5]);

  return DeviceTreeParser((const uint8_t*)dts, total_size, off_struct, off_strings, off_mem_reserved_map, version);
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
  offset = align_pointer(offset + property_size, alignof(uint32_t));

  return offset;
}

size_t DeviceTreeParser::skip_node(size_t offset) const {
  KASSERT(get_uint32(offset) == DTB_BEGIN_NODE);

  // Ignore BEGIN_NODE token
  offset += sizeof(uint32_t);

  // And ignore the node's name
  const size_t name_size = libk::strlen(get_string(offset)) + 1;  // We count the \000 at the end
  offset = align_pointer(offset + name_size, alignof(uint32_t));

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
