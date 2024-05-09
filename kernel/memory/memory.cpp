#include "memory.hpp"
#include <libk/log.hpp>

#include "boot/kernel_dt.hpp"
#include "memory/page_alloc_list.hpp"
#include "memory/page_builder.hpp"

MMUTable _tbl;
PageAllocList _page_alloc;
MemoryPageBuilder _p_builder;

VirtualPA mmu_resolve_pa(void*, PhysicalPA page_address) {
  return page_address + KERNEL_BASE;
}

PhysicalPA mmu_resolve_va(void*, VirtualPA page_address) {
  return page_address - KERNEL_BASE;
}

VirtualPA mmu_alloc_page(void*) {
  PhysicalPA addr = -1;

  if (!_page_alloc.fresh_page(&addr)) {
    libk::panic("[KernelMemory] Unable to allocate a page for the KERNEL MMU.");
  }

  const VirtualPA va = mmu_resolve_pa(nullptr, addr);
  zero_pages(va, 1);

  return va;
}

void mmu_free_page(void*, VirtualPA page_address) {
  const PhysicalPA addr = mmu_resolve_va(nullptr, page_address);

  _page_alloc.free_page(addr);
}

void KernelMemory::init() {
  /* Set up the PageAllocList */
  constexpr size_t linear_allocator_size = 1;  //<! Number of page reserved by the internal Linear Memory Allocator
  _page_alloc = PageAllocList(linear_allocator_size);

  /* Set up the MMUTable */
  _tbl = {
      .kind = MMUTable::Kind::Kernel,
      .pgd = _init_data.pgd,
      .asid = 0,
      .handle = nullptr,
      .alloc = &mmu_alloc_page,
      .free = &mmu_free_page,
      .resolve_pa = &mmu_resolve_pa,
      .resolve_va = &mmu_resolve_va,
  };

  /* Set up the page builder */
  _p_builder = MemoryPageBuilder(_page_alloc, &_tbl);
}

VirtualPA KernelMemory::get_heap_start() {
  return HEAP_MEMORY;
}

VirtualPA KernelMemory::get_heap_end() {
  return _p_builder.get_heap_end();
}

size_t KernelMemory::get_heap_byte_size() {
  return _p_builder.get_heap_size();
}

VirtualPA KernelMemory::change_heap_end(long byte_offset) {
  return _p_builder.change_heap_end(byte_offset);
}

size_t KernelMemory::get_memory_overhead() {
  return _lin_alloc->nb_allocated * PAGE_SIZE;
}

bool KernelMemory::new_page(MemoryPage* page) {
  return _p_builder.create_custom_page(page);
}
