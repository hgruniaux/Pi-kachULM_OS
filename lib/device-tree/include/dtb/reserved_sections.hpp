#pragma once

#include <iterator>
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
  explicit ReservedSections(const DeviceTreeParser* parser) : m_p(parser) {}

  [[nodiscard]] MemorySectionIterator begin() const { return MemorySectionIterator(m_p, m_p->get_reserved_memory_offset()); }
  [[nodiscard]] MemorySectionIterator end() const { return MemorySectionIterator(m_p); }

 private:
  const DeviceTreeParser* m_p;
};

static_assert(std::forward_iterator<MemorySectionIterator>);
