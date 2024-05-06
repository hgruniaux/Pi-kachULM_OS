#pragma once

#include "node.hpp"
#include "parser.hpp"
#include "reserved_sections.hpp"

class DeviceTree {
 public:
  DeviceTree() = default;
  explicit DeviceTree(uintptr_t dts);

  [[nodiscard]] bool is_status_okay() const { return m_p.is_initialized(); }

  [[nodiscard]] inline uint32_t get_version() const { return m_p.get_version(); }

  [[nodiscard]] bool get_reserved_sections(ReservedSections* res) const;

  [[nodiscard]] bool get_root(Node* node) const;

  [[nodiscard]] bool find_node(libk::StringView path, Node* node) const;

  [[nodiscard]] bool find_property(libk::StringView path, Property* property) const;

 private:
  DeviceTreeParser m_p;
};
