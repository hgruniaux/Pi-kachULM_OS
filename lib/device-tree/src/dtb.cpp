#include "dtb/dtb.hpp"
#include "dtb/node.hpp"
#include "libk/string_view.hpp"

bool DeviceTree::find_node(libk::StringView path, Node* node) const {
  Node current_node = {};

  if (!get_root(&current_node)) {
    return false;
  }

  size_t begin = 0;

  if (!path.is_empty() && path[0] == '/') {
    begin++;
  }

  while (begin < path.get_length()) {
    const auto* next_delim = path.find('/', begin);
    const size_t node_name_length = next_delim - path.begin() - begin;

    // Next node name is in path[0] ... path[node_name_length]
    if (current_node.find_child(libk::StringView(path.begin() + begin, node_name_length), &current_node)) {
      begin += node_name_length;
      if (begin < path.get_length()) {
        begin++;  // Skip the '/'
      }
    } else {
      return false;
    }
  }

  *node = current_node;
  return true;
}

bool DeviceTree::find_property(libk::StringView path, Property* property) const {
  Node node = {};

  const auto* last_delim = path.rfind('/');

  if (last_delim == path.end()) {
    if (!get_root(&node)) {
      return false;
    }

    return node.find_property(path, property);
  } else {
    if (!find_node({path.begin(), last_delim}, &node)) {
      return false;
    }

    return node.find_property(last_delim + 1, property);
  }
}

bool DeviceTree::get_reserved_sections(ReservedSections* res) const {
  if (!is_status_okay()) {
    return false;
  }

  *res = ReservedSections(&m_p);
  return true;
}

bool DeviceTree::get_root(Node* node) const {
  if (!is_status_okay()) {
    return false;
  }

  *node = Node(&m_p, m_p.get_struct_offset());
  return true;
}

DeviceTree::DeviceTree(uintptr_t dts) {
  (void)DeviceTreeParser::from_memory(dts, &m_p);
}
