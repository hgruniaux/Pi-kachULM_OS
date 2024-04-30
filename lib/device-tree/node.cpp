#include "dtb/node.hpp"

#include "dtb/parser.hpp"
#include "libk/assert.hpp"
#include "libk/string.hpp"
#include "libk/utils.hpp"
#include "utils.hpp"

static constexpr size_t max_val = -1;

/*
 * Property class
 */
bool Property::is_string() const {
  for (size_t i = 0; i < length - 1; ++i) {
    if (data[i] <= 0x1f || data[i] >= 0x7f) {
      return false;
    }
  }

  return data[length - 1] == '\0';
}

bool Property::get_u32_at(size_t* index, uint64_t* value) const {
  if (index == nullptr || *index + sizeof(uint32_t) > length) {
    return false;
  }

  if (value != nullptr) {
    *value = libk::from_be(*(const uint32_t*)(data + *index));
  }

  *index += sizeof(uint32_t);
  return true;
}

bool Property::get_u64_at(size_t* index, uint64_t* value) const {
  if (index == nullptr || *index + sizeof(uint64_t) > length) {
    return false;
  }

  if (value != nullptr) {
    *value = libk::from_be(*(const uint64_t*)(data + *index));
  }

  *index += sizeof(uint64_t);
  return true;
}

bool Property::get_variable_int(size_t* index, uint64_t* value, bool is_u64_integer) const {
  if (is_u64_integer) {
    return get_u64_at(index, value);
  } else {
    return get_u32_at(index, value);
  }
}

libk::Option<uint32_t> Property::get_u32() const {
  if (length != sizeof(uint32_t))
    return {};

  const uint32_t value = *((const uint32_t*)data);
  return libk::from_be(value);
}

libk::Option<uint64_t> Property::get_u64() const {
  if (length != sizeof(uint64_t))
    return {};

  const uint64_t value = *((const uint64_t*)data);
  return libk::from_be(value);
}

[[nodiscard]] libk::Option<uint64_t> Property::get_u32_or_u64() const {
  if (length == sizeof(uint32_t)) {
    const uint32_t value = *((const uint32_t*)data);
    return libk::from_be(value);
  }

  if (length == sizeof(uint64_t)) {
    const uint64_t value = *((const uint64_t*)data);
    return libk::from_be(value);
  }

  return {};
}

/*
 * PropertyIterator Class
 */

bool find_next_property(const DeviceTreeParser* m_p, size_t* offset) {
  while (true) {
    const uint32_t token = m_p->get_uint32(*offset);
    switch (token) {
      case DTB_BEGIN_NODE: {
        // Skip our children
        *offset = m_p->skip_node(*offset);
        break;
      }
      case DTB_PROP: {
        // Property found !
        return true;
      }

      case DTB_NOP: {
        // Skip this
        *offset += sizeof(uint32_t);
        break;
      }
      case DTB_END_NODE: {
        // End of out node -> nothing to be found
        return false;
      }
      default: {
        KASSERT(false);
        // TODO : Fix this with a message
        // LOG_CRITICAL("Unrecognized token in Device Tree Blob at offset {} : {}.", offset, token);
      }
    }
  }
}

PropertyIterator::PropertyIterator(const DeviceTreeParser* parser, size_t offset) : m_p(parser) {
  KASSERT(m_p != nullptr);
  m_off = offset;

  if (!find_next_property(m_p, &m_off)) {
    m_off = max_val;
  }
}

PropertyIterator::PropertyIterator(const DeviceTreeParser* parser) : m_p(parser), m_off(max_val) {}

PropertyIterator::element_type PropertyIterator::operator*() const {
  KASSERT(m_off != max_val);
  KASSERT(m_p != nullptr);
  KASSERT(m_p->get_uint32(m_off) == DTB_PROP);

  // Get property length
  const uint32_t prop_length = m_p->get_uint32(m_off + sizeof(uint32_t));

  // Get property string offset
  const size_t property_name_offset = m_p->get_uint32(m_off + 2 * sizeof(uint32_t));
  const char* prop_name = m_p->get_string(m_p->string_offset + property_name_offset);

  // Get property data
  const char* prop_data = m_p->get_string(m_off + 3 * sizeof(uint32_t));
  return {prop_name, prop_length, prop_data};
}

PropertyIterator& PropertyIterator::operator++() {
  KASSERT(m_p != nullptr);
  if (m_off == max_val) {
    return *this;
  }

  m_off = m_p->skip_property(m_off);

  if (!find_next_property(m_p, &m_off)) {
    m_off = max_val;
  }

  return *this;
}

/*
 * NodeIterator Class
 */

bool find_next_node(const DeviceTreeParser* m_p, size_t* offset) {
  while (true) {
    const uint32_t token = m_p->get_uint32(*offset);
    switch (token) {
      case DTB_BEGIN_NODE: {
        // Find one of our child
        return true;
      }
      case DTB_PROP: {
        // Find one of our properties
        *offset = m_p->skip_property(*offset);
        break;
      }
      case DTB_NOP: {
        // Skip this
        *offset += sizeof(uint32_t);
        break;
      }
      case DTB_END_NODE: {
        // End of out node -> nothing to be found
        return false;
      }
      default: {
        KASSERT(false);
        // TODO : Fix this with a message
        // LOG_CRITICAL("Unrecognized token in Device Tree Blob at offset {} : {}.", offset, token);
      }
    }
  }
}

NodeIterator::NodeIterator(const DeviceTreeParser* parser, size_t offset) : m_p(parser) {
  KASSERT(m_p != nullptr);

  if (find_next_node(m_p, &offset)) {
    m_off = offset;
  } else {
    m_off = max_val;
  }
}

NodeIterator::NodeIterator(const DeviceTreeParser* parser) : m_p(parser), m_off(max_val) {}

NodeIterator::element_type NodeIterator::operator*() const {
  KASSERT(m_off != max_val);
  KASSERT(m_p != nullptr);
  KASSERT(m_p->get_uint32(m_off) == DTB_BEGIN_NODE);

  return Node(m_p, m_off);
}

NodeIterator& NodeIterator::operator++() {
  KASSERT(m_p != nullptr);

  if (m_off == max_val) {
    return *this;
  }

  m_off = m_p->skip_node(m_off);

  if (!find_next_node(m_p, &m_off)) {
    m_off = max_val;
  }

  return *this;
}

/*
 * Node Class
 */

Node::Node(const DeviceTreeParser* parser, size_t offset) : m_p(parser) {
  KASSERT(m_p->get_uint32(offset) == DTB_BEGIN_NODE);
  offset += sizeof(uint32_t);

  m_name = m_p->get_string(offset);
  const size_t name_size = libk::strlen(m_name) + 1;  // We count the \000 at the end

  m_off = libk::align(offset + name_size, alignof(uint32_t));
}
