#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>

#include "libk/string.hpp"
#include "libk/string_view.hpp"
#include "node.hpp"
#include "parser.hpp"

struct MemorySection {
  uint64_t address;
  uint64_t size;
};

class MemorySectionIterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using element_type = MemorySection;
  using difference_type = std::ptrdiff_t;

  MemorySectionIterator() = default;
  explicit MemorySectionIterator(const DeviceTreeParser* parser, size_t offset) : m_p(parser), m_off(offset) {}
  explicit MemorySectionIterator(const DeviceTreeParser* parser);

  element_type operator*() const;

  MemorySectionIterator& operator++();
  MemorySectionIterator operator++(int) {
    MemorySectionIterator old = *this;
    ++(*this);
    return old;
  }

  bool operator==(const MemorySectionIterator& b) const { return m_p == b.m_p && m_off == b.m_off; }

 private:
  const DeviceTreeParser* m_p;
  size_t m_off;
};

class ReservedSections {
 public:
  [[nodiscard]] MemorySectionIterator begin() const { return MemorySectionIterator(m_p, m_p->reserved_memory_offset); }
  [[nodiscard]] MemorySectionIterator end() const { return MemorySectionIterator(m_p); }

 protected:
  friend DeviceTree;
  explicit ReservedSections(const DeviceTreeParser* parser) : m_p(parser) {}

  const DeviceTreeParser* m_p;
};

class DeviceTree {
 public:
  explicit DeviceTree(const void* dts) : m_p(DeviceTreeParser::from_memory(dts)) {}
  [[nodiscard]] bool is_status_okay() const { return m_p.is_status_okay(); }

  [[nodiscard]] uint32_t get_version() const { return m_p.version; }

  [[nodiscard]] ReservedSections get_reserved_sections() const { return ReservedSections(&m_p); }

  [[nodiscard]] Node get_root() const { return Node(&m_p, m_p.struct_offset); }

  [[nodiscard]] bool find_node(libk::StringView path, Node* node) const;

  [[nodiscard]] bool find_property(libk::StringView path, Property* property) const;

 private:
  const DeviceTreeParser m_p;
};

static_assert(std::forward_iterator<MemorySectionIterator>);
