#include "memory_chunk.hpp"

#include <algorithm>
#include <libk/log.hpp>
#include <libk/string.hpp>
#include <libk/utils.hpp>

#include "boot/mmu_utils.hpp"
#include "memory/kernel_internal_memory.hpp"
#include "memory/process_memory.hpp"

MemoryChunk::MemoryChunk(size_t nb_pages)
    : _nb_pages(nb_pages),
      _pas(new PhysicalPA[_nb_pages]),
      _kernel_va(memory_impl::allocate_pages_section(_nb_pages, _pas)) {
  if (_kernel_va == 0) {
    LOG_ERROR("[MemoryChunk] Failed to allocate {} pages.", nb_pages);
  }
}

MemoryChunk::~MemoryChunk() {
  // Free all mapping in processes
  for (const auto proc : _proc) {
    proc.proc->unmap_memory(proc.chunk_start);  // <- proc will be removed from the list by the unregister call
  }

  KASSERT(_proc.is_empty());

  // Free the kernel memory
  memory_impl::free_section(_nb_pages, _kernel_va, _pas);

  // Free the array & invalidate object.
  delete[] _pas;
  _pas = nullptr;
  _kernel_va = 0;
}

size_t MemoryChunk::write(size_t byte_offset, const void* data, size_t data_byte_length) {
  if (byte_offset >= get_byte_size() || _kernel_va == 0) {
    return 0;
  }

  void* begin = (void*)(_kernel_va + byte_offset);
  const size_t available_to_write = get_byte_size() - byte_offset;
  const size_t to_write = libk::min(available_to_write, data_byte_length);

  libk::memcpy(begin, data, to_write);

  return to_write;
}

size_t MemoryChunk::read(size_t byte_offset, void* data, size_t data_byte_length) const {
  if (byte_offset >= get_byte_size() || _kernel_va == 0) {
    return 0;
  }

  const auto* begin = (const void*)(_kernel_va + byte_offset);
  const size_t available_to_read = get_byte_size() - byte_offset;
  const size_t to_read = libk::min(available_to_read, data_byte_length);

  libk::memcpy(data, begin, to_read);

  return to_read;
}

size_t MemoryChunk::get_byte_size() const {
  return _nb_pages * PAGE_SIZE;
}

VirtualPA MemoryChunk::end_address(VirtualPA start_address) {
  return start_address + get_byte_size() - PAGE_SIZE;
}

void MemoryChunk::register_mapping(ProcessMemory* proc_mem, VirtualPA start_addr) {
  _proc.emplace_back(start_addr, proc_mem);
}

void MemoryChunk::unregister_mapping(ProcessMemory* proc_mem) {
  auto it = std::find_if(_proc.begin(), _proc.end(), [proc_mem](const auto& proc) { return proc_mem == proc.proc; });

  if (it == std::end(_proc)) {
    libk::panic("Trying to remove an unknown binding Process <-> MemoryChunk !");
  }

  _proc.erase(it);
}

bool MemoryChunk::is_status_okay() const {
  return _kernel_va != 0;
}

size_t MemoryChunk::get_page_byte_size() {
  return PAGE_SIZE;
}
