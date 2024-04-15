#include "libk/bit_array.hpp"
#include "libk/string.hpp"

namespace libk {
BitArray::BitArray(void* begin, size_t taille) : m_array((uint64_t*)begin) {
  if (begin != nullptr) {
    uint8_t value = 0xff;
    libk::memset(m_array, value, taille / 8);
    set_bit((size_t)0, false);
  }
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

}  // namespace libk