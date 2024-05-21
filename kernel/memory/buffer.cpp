#include "buffer.hpp"
#include <algorithm>
#include "boot/mmu_utils.hpp"
#include "hardware/mailbox.hpp"
#include "kernel_internal_memory.hpp"
#include "process_memory.hpp"

Buffer::Buffer(uint32_t byte_size) {
  const size_t nb_pages = libk::div_round_up(byte_size, PAGE_SIZE);
  if (!memory_impl::allocate_buffer_pa(nb_pages, &buffer_pa_start, &buffer_pa_end)) {
    libk::panic("Error Allocating Buffer.");
  }

  LOG_DEBUG("We have a Buffer {:#x} -> {:#x}", buffer_pa_start, buffer_pa_end);
  kernel_va = memory_impl::map_buffer(buffer_pa_start, buffer_pa_end);
  LOG_DEBUG("Mapped from {:#x}", kernel_va);
}

Buffer::~Buffer() {
  // Free all mapping in processes
  for (const auto proc : _proc) {
    proc.proc->unmap_memory(proc.buffer_start);  // <- proc will be removed from the list by the unregister call
  }

  KASSERT(_proc.is_empty());

  memory_impl::unmap_buffer(kernel_va, kernel_va + buffer_pa_end - buffer_pa_start);
  memory_impl::free_buffer_pa(buffer_pa_start, buffer_pa_end);
}

size_t Buffer::get_byte_size() const {
  return buffer_pa_end - buffer_pa_start + PAGE_SIZE;
}

void* Buffer::get() const {
  return (void*)kernel_va;
}

DMA::Address Buffer::get_dma_address() {
  return DMA::get_dma_bus_address(kernel_va, false);
}

VirtualPA Buffer::end_address(VirtualPA start_address) {
  return start_address + get_byte_size() - PAGE_SIZE;
}

void Buffer::register_mapping(ProcessMemory* proc_mem, VirtualPA start_addr) {
  _proc.emplace_back(start_addr, proc_mem);
}

void Buffer::unregister_mapping(ProcessMemory* proc_mem) {
  auto it = std::find_if(_proc.begin(), _proc.end(), [proc_mem](const auto& proc) { return proc_mem == proc.proc; });

  if (it == std::end(_proc)) {
    libk::panic("Trying to remove an unknown binding Process <-> MemoryChunk !");
  }

  _proc.erase(it);
}
