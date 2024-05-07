#pragma once

#include <cstddef>
#include <cstdint>

namespace libk {
class LinearAllocator {
 public:
  LinearAllocator() = default;
  explicit LinearAllocator(void* begin_section, size_t section_bytes_size);

  void* malloc(size_t byte_size, size_t align);

  template <class T>
  T* new_class() {
    return (T*)malloc(sizeof(T), alignof(T));
  }

 private:
  uintptr_t _beg = 0;
  uintptr_t _max_address = 0;
};

}  // namespace libk
