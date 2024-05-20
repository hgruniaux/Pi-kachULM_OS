#pragma once

#include "dtb/node.hpp"
#include "libk/linear_allocator.hpp"
#include "memory/page_alloc.hpp"

class PageAllocList {
 public:
  PageAllocList() = default;
  PageAllocList(libk::LinearAllocator& mem_alloc, size_t contiguous_res_bytes);

  bool fresh_page(PhysicalPA* addr);

  void free_page(PhysicalPA addr);

  void mark_as_used_range(PhysicalPA start, PhysicalPA end);

  PhysicalPA get_reserved_start() const { return contiguous_res_start; }
  PhysicalPA get_reserved_stop() const { return contiguous_res_stop; }

 private:
  struct AllocList {
    PhysicalPA section_start;
    PhysicalPA section_stop;
    PageAlloc alloc;
    AllocList* next;
  };

  PhysicalPA contiguous_res_start;
  PhysicalPA contiguous_res_stop;

  AllocList* _list = nullptr;

  void add_allocator(libk::LinearAllocator& mem_alloc,
                     PhysicalPA page_start,
                     PhysicalPA page_end,
                     size_t nb_pages,
                     uintptr_t array);
  bool parse_memory_reg(libk::LinearAllocator& mem_alloc, Property prop, PageAllocList* list, size_t contiguous_res);
};
