#pragma once
#include <cstddef>
#include <cstdint>

namespace libk {

class BitArray {
 public:
  // Constructors
  explicit BitArray(void* begin, size_t bit_taille);
  bool get_bit(size_t index) const;
  void set_bit(size_t index, bool value);
  // Operator
  bool operator[](size_t index) const { return get_bit(index); }

 private:
  uint64_t* m_array;
};

};  // namespace libk