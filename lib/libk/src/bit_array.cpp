#include "libk/bit_array.hpp"
#include "libk/string.hpp"

namespace libk {
BitArray::BitArray(void* begin, size_t taille) : m_array((uint8_t*)begin) {
  if (begin != nullptr) {
    bool value = 1;
    libk::memset(m_array, value, taille / 8);
    libk::memset(m_array, 0, 1);
  }
}

bool BitArray::get_bit(size_t index) const {
  return m_array[index / (8 * sizeof(uint8_t))] & (1u << (index % (8 * sizeof(uint8_t))));
}

void BitArray::set_bit(size_t index, bool value) {
  const uint8_t v = m_array[index / (8 * sizeof(uint8_t))];
  if (value) {
    m_array[index / (8 * sizeof(uint8_t))] = v | (1u << (index % (8 * sizeof(uint8_t))));
  } else {
    m_array[index / (8 * sizeof(uint8_t))] = v & ~(1u << (index % (8 * sizeof(uint8_t))));
  }
}

}