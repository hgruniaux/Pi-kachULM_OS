#include "dtb/parser.hpp"

#include <libk/assert.hpp>
#include <libk/string.hpp>
#include "libk/log.hpp"
#include "utils.hpp"

bool DeviceTreeParser::from_memory(uintptr_t dts, DeviceTreeParser* dt_parser) {
  if (dts == 0 || libk::align_to_next(dts, alignof(uint64_t)) != dts) {
    // Unaligned DeviceTree blob
    return false;
  }

  const auto* header = (const uint32_t*)dts;
  if (libk::from_be(header[0]) != DTB_MAGIC) {
    // Not a DeviceTree blob !
    return false;
  }

  const uint32_t off_struct = libk::from_be(header[2]);
  if (libk::align_to_next(off_struct, alignof(uint32_t)) != off_struct) {
    // Unaligned property section
    return false;
  }

  const uint32_t off_mem_reserved_map = libk::from_be(header[4]);
  if (libk::align_to_next(off_mem_reserved_map, alignof(uint64_t)) != off_mem_reserved_map) {
    // Unaligned reserved memory section
    return false;
  }

  dt_parser->_dts = dts;
  dt_parser->_struct_ofs = off_struct;
  dt_parser->_string_ofs = libk::from_be(header[3]);
  dt_parser->_res_mem_ofs = off_mem_reserved_map;
  dt_parser->_version = libk::from_be(header[5]);

  return true;
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
  offset = libk::align_to_next(offset + property_size, alignof(uint32_t));

  return offset;
}

size_t DeviceTreeParser::skip_node(size_t offset) const {
  KASSERT(get_uint32(offset) == DTB_BEGIN_NODE);

  // Ignore BEGIN_NODE token
  offset += sizeof(uint32_t);

  // And ignore the node's name
  const size_t name_size = libk::strlen(get_string(offset)) + 1;  // We count the \000 at the end
  offset = libk::align_to_next(offset + name_size, alignof(uint32_t));

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
        LOG_ERROR("Unrecognized token in Device Tree Blob at offset {} : {}.", offset, token);
        libk::panic("[DeviceTree] Unable to parse device tree.");
      }
    }
  }
}
