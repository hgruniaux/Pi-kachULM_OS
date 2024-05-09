#pragma once

#include <cstddef>
#include <cstdint>
#include "memory.hpp"
#include "page_alloc_list.hpp"

class MemoryPageBuilder {
 public:
  MemoryPageBuilder() = default;
  MemoryPageBuilder(PageAllocList page_alloc, MMUTable* table);

  /* Custom Kernel Pages */
  bool create_custom_page(MemoryPage* page);

  void unregister_page(MemoryPage page);

  /* Kernel Heap */
  VirtualPA change_heap_end(long byte_offset);

  VirtualPA get_heap_end() const { return HEAP_MEMORY + _heap_size; }

  size_t get_heap_size() const { return _heap_size; }

 private:
  size_t _custom_index = 0;
  size_t _heap_size = 0;

  PageAllocList _alloc;
  MMUTable* _tbl = nullptr;

  static PhysicalPA mmu_resolve_va(VirtualPA va);
};
