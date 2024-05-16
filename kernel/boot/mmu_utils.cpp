#include "boot/mmu_utils.hpp"

// Force init_data to be in the .data segment (and not .bss)
MMUInitData _init_data = {0x1, {}, {}, 0x2, 0x3, 0x4, 0x5, 0x6};

void zero_pages(VirtualPA pages, size_t nb_pages) {
  auto* const page_ptr = (uint64_t*)pages;
  for (size_t i = 0; i < nb_pages * PAGE_SIZE / sizeof(uint64_t); ++i) {
    page_ptr[i] = 0;
  }
}

bool allocate_pages(LinearPageAllocator* alloc, size_t nb_pages, PhysicalPA* page) {
  const uintptr_t cur_page = alloc->first_page + PAGE_SIZE * alloc->nb_allocated;

  if (cur_page + nb_pages * PAGE_SIZE >= alloc->upper_bound) {
    return false;
  }

  if (page != nullptr) {
    *page = cur_page;
  }

  alloc->nb_allocated += nb_pages;
  return true;
}
