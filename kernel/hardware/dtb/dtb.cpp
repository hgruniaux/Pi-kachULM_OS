#include "dtb.hpp"

#include <cstddef>
#include <cstdint>

#include "../../debug.hpp"
#include "../uart.hpp"
#include "node.hpp"
#include "parser.hpp"

static constexpr size_t max_value = -1;

MemorySectionIterator::MemorySectionIterator(const DeviceTreeParser* parser) : m_p(parser), m_off(max_value) {}

MemorySectionIterator::element_type MemorySectionIterator::operator*() const {
  KASSERT(m_p != nullptr);
  KASSERT(m_off != max_value);

  const uint64_t address = m_p->get_uint64(m_off);
  const uint64_t size = m_p->get_uint64(m_off + sizeof(uint64_t));

  return {address, size};
}

MemorySectionIterator& MemorySectionIterator::operator++() {
  const uint64_t tmp_address = m_p->get_uint64(m_off + 2 * sizeof(uint64_t));
  const uint64_t tmp_size = m_p->get_uint64(m_off + 3 * sizeof(uint64_t));

  if (tmp_address == 0 && tmp_size == 0) {
    m_off = max_value;
  } else {
    m_off += 2 * sizeof(uint64_t);
  }

  return *this;
}

bool DeviceTree::find_node(const char* path, size_t path_length, Node* node) const {
  Node current_node = get_root();
  size_t begin = 0;

  if (path[0] == '/') {
    begin++;
  }

  while (begin < path_length) {
    const char* next_delim = strchrnul(path + begin, '/');
    const size_t node_name_length = next_delim - path - begin - 1;

    // Next node name is in path[0] ... path[node_name_length]
    if (current_node.find_child(path + begin, node_name_length, &current_node)) {
      begin += node_name_length + 1;
      if (path[begin] != '\000') {
        begin++;  // Skip the '/'
      }
    } else {
      return false;
    }
  }

  *node = current_node;
  return true;
}

bool DeviceTree::find_property(const char* path, Property* property) const {
  const char* last_delim = strrchr(path, '/');

  if (last_delim == nullptr) {
    return get_root().find_property(path, property);
  }

  Node node = {};
  if (find_node(path, last_delim - path, &node)) {
    return node.find_property(last_delim + 1, property);
  }

  return false;
}
