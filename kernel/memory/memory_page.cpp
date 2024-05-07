#include "memory_page.hpp"
#include <libk/log.hpp>
#include <libk/string.hpp>
#include "boot/mmu_utils.hpp"
#include "memory/memory_page_builder.hpp"

MemoryPage::MemoryPage(PhysicalPA pa, VirtualPA va, MemoryPageBuilder* builder)
    : _pa(pa), _kernel_va(va), _builder(builder) {
  zero_pages(_kernel_va, 1);
}

size_t MemoryPage::write(size_t byte_offset, const void* data, size_t data_byte_length) const {
  if (byte_offset >= PAGE_SIZE || _kernel_va == 0) {
    return 0;
  }

  void* begin = (void*)(_kernel_va + byte_offset);
  const size_t available_to_write = PAGE_SIZE - byte_offset;
  const size_t to_write = libk::min(available_to_write, data_byte_length);

  libk::memcpy(begin, data, to_write);

  return to_write;
}

size_t MemoryPage::read(size_t byte_offset, void* data, size_t data_byte_length) const {
  if (byte_offset >= PAGE_SIZE || _kernel_va == 0) {
    return 0;
  }

  const void* begin = (const void*)(_kernel_va + byte_offset);
  const size_t available_to_read = PAGE_SIZE - byte_offset;
  const size_t to_read = libk::min(available_to_read, data_byte_length);

  libk::memcpy(data, begin, to_read);

  return to_read;
}
void MemoryPage::free() {
  _builder->unregister_page(_pa, _kernel_va);
  _pa = 0;
  _kernel_va = 0;
}
