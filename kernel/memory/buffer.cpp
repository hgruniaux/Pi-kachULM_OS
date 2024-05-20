#include "buffer.hpp"
#include "boot/mmu_utils.hpp"
#include "hardware/mailbox.hpp"
#include "kernel_internal_memory.hpp"

Buffer::Buffer(uint32_t byte_size)
    : nb_pages(libk::div_round_up(byte_size, PAGE_SIZE)),
      buffer_pa(memory_impl::allocate_buffer_pa(nb_pages)),
      kernel_va(memory_impl::map_buffer(buffer_pa, nb_pages)) {}

Buffer::~Buffer() {
  memory_impl::unmap_buffer(kernel_va, nb_pages);
  memory_impl::free_buffer_pa(buffer_pa, nb_pages);
}

size_t Buffer::get_byte_size() const {
  return nb_pages * PAGE_SIZE;
}

void* Buffer::get() const {
  return (void*)kernel_va;
}

DMA::Address Buffer::get_dma_address() {
  return DMA::get_dma_bus_address(kernel_va, false);
}
