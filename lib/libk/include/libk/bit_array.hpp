#pragma once
#include <cstddef>
#include <cstdint>

namespace libk {

class BitArray {
 public:
  // Constructors
  explicit BitArray() = default;
  explicit BitArray(void* begin, size_t bytes_size);

  // Modifiers
  bool get_bit(size_t index) const;
  void set_bit(size_t index, bool value);
  // Operator
  bool operator[](size_t index) const { return get_bit(index); }

  void fill_array(bool value);

 private:
  uint64_t* m_array;
  size_t m_bytes_size;
};

};  // namespace libk
