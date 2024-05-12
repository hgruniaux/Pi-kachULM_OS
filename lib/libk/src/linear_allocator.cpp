#include "libk/linear_allocator.hpp"
#include <cstdint>
#include "libk/utils.hpp"

namespace libk {
LinearAllocator::LinearAllocator(uintptr_t begin_section, size_t section_bytes_size)
    : _beg(begin_section), _max_address(begin_section + section_bytes_size) {}

void* LinearAllocator::malloc(size_t byte_size, size_t align) {
  if (_beg == 0) {
    return nullptr;
  }

  const uintptr_t aligned_beg = libk::align_to_next(_beg, align);
  const uintptr_t next_beg = aligned_beg + byte_size;

  if (next_beg >= _max_address) {
    return nullptr;
  }

  const auto block_start = (void*)aligned_beg;
  _beg = next_beg;

  return block_start;
}

}  // namespace libk
