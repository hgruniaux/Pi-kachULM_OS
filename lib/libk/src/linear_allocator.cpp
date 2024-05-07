#include "libk/linear_allocator.hpp"
#include <cstdint>
#include "libk/utils.hpp"

namespace libk {
LinearAllocator::LinearAllocator(void* begin_section, size_t section_bytes_size)
    : _beg((uintptr_t)begin_section), _max_address(_beg + section_bytes_size) {}

void* LinearAllocator::malloc(size_t byte_size, size_t align) {
  const uintptr_t aligned_beg = libk::align_to_next(_beg, align);
  const uintptr_t next_beg = aligned_beg + byte_size;

  if (next_beg >= _max_address) {
    return 0;
  }

  const auto block_start = (void*)aligned_beg;
  _beg = next_beg;

  return block_start;
}

}  // namespace libk
