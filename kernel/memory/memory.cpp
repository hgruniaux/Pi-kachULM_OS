#include "memory.hpp"

#include "boot/mmu_utils.hpp"
#include "memory/heap_manager.hpp"
#include "memory/kernel_internal_memory.hpp"

// The heap manager could not be a static variable, otherwise the HeapManager constructor
// will be called just before kmain() but after KernelMemory initialization.
// So to be sure that the HeapManager is not initialized after KernelMemory::init(), we use
// lazy static initialization inside this util function.
static HeapManager& get_heap_manager() {
  static HeapManager builder;
  return builder;
}

bool KernelMemory::init() {
  if (!memory_impl::init()) {
    return false;
  }

  /* Set up the page builder */
  get_heap_manager() = HeapManager(HeapManager::Kind::Kernel, memory_impl::get_kernel_tbl());

  return true;
}

VirtualPA KernelMemory::get_heap_start() {
  return HEAP_MEMORY;
}

VirtualPA KernelMemory::get_heap_end() {
  return get_heap_manager().get_heap_end();
}

size_t KernelMemory::get_heap_byte_size() {
  return get_heap_manager().get_heap_byte_size();
}

VirtualPA KernelMemory::change_heap_end(long byte_offset) {
  return get_heap_manager().change_heap_end(byte_offset);
}

size_t KernelMemory::get_memory_overhead() {
  return _lin_alloc->nb_allocated * PAGE_SIZE;
}

PhysicalAddress KernelMemory::get_physical_vc_address(VirtualAddress vc_addr) {
  return vc_addr - VC_MEMORY;
}

VirtualAddress KernelMemory::get_virtual_vc_address(PhysicalAddress vc_addr) {
  return vc_addr + VC_MEMORY;
}

VirtualAddress KernelMemory::get_fs_address() {
  return RAM_FS_MEMORY;
}
