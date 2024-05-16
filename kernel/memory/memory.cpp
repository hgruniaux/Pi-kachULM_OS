#include "memory.hpp"

#include "boot/mmu_utils.hpp"
#include "memory/heap_manager.hpp"
#include "memory/kernel_internal_memory.hpp"

HeapManager _p_builder;

void KernelMemory::init() {
  memory_impl::init();

  /* Set up the page builder */
  _p_builder = HeapManager(HeapManager::Kind::Kernel, memory_impl::get_kernel_tbl());
}

VirtualPA KernelMemory::get_heap_start() {
  return HEAP_MEMORY;
}

VirtualPA KernelMemory::get_heap_end() {
  return _p_builder.get_heap_end();
}

size_t KernelMemory::get_heap_byte_size() {
  return _p_builder.get_heap_byte_size();
}

VirtualPA KernelMemory::change_heap_end(long byte_offset) {
  return _p_builder.change_heap_end(byte_offset);
}

size_t KernelMemory::get_memory_overhead() {
  return _lin_alloc->nb_allocated * PAGE_SIZE;
}

PhysicalAddress KernelMemory::get_physical_vc_address(VirtualAddress vc_addr) {
  return vc_addr - VC_MEMORY + _init_data.vc_offset;
}

VirtualAddress KernelMemory::get_virtual_vc_address(PhysicalAddress vc_addr) {
  return vc_addr - _init_data.vc_offset + VC_MEMORY;
}
