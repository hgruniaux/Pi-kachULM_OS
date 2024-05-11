#pragma once

#include "libk/linear_allocator.hpp"
#include "memory/page_alloc.hpp"

class PageAllocList {
 public:
  PageAllocList() = default;
  PageAllocList(size_t internal_memory_size);

  void add_allocator(PhysicalPA page_start, PhysicalPA page_end, size_t nb_pages, uintptr_t array);

  bool fresh_page(PhysicalPA* addr);

  void free_page(PhysicalPA addr);

  void mark_as_used_range(PhysicalPA start, PhysicalPA end);

 private:
  struct AllocList {
    PhysicalPA section_start;
    PhysicalPA section_stop;
    PageAlloc alloc;
    AllocList* next;
  };

  libk::LinearAllocator _mem_alloc;
  AllocList* _list = nullptr;
};
