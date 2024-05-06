#include "libk/bit_array.hpp"
#include "libk/string.hpp"

namespace libk {
BitArray::BitArray(void* begin, size_t bytes_size) : m_array((uint64_t*)begin), m_bytes_size(bytes_size) {
  fill_array(false);
}

bool BitArray::get_bit(size_t index) const {
  return m_array[index / (8 * sizeof(uint64_t))] & (1ull << (index % (8 * sizeof(uint64_t))));
}

void BitArray::set_bit(size_t index, bool value) {
  const uint64_t v = m_array[index / (8 * sizeof(uint64_t))];
  if (value) {
    m_array[index / (8 * sizeof(uint64_t))] = v | (1ull << (index % (8 * sizeof(uint64_t))));
  } else {
    m_array[index / (8 * sizeof(uint64_t))] = v & ~(1ull << (index % (8 * sizeof(uint64_t))));
  }
}

void BitArray::fill_array(bool value) {
  const uint8_t int_value = value ? 0xff : 0x00;
  libk::memset(m_array, int_value, m_bytes_size);
}
}  // namespace libk
