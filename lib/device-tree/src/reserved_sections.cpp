#include "dtb/reserved_sections.hpp"

#include <libk/assert.hpp>

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
