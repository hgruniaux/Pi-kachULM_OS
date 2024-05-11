#pragma once

#include <cstddef>
#include <cstdint>
#include "memory/memory.hpp"
#include "memory/page_alloc_list.hpp"

class MemoryPageBuilder {
 public:
  MemoryPageBuilder() = default;
  MemoryPageBuilder(PageAllocList page_alloc, MMUTable* table);

  /* Kernel Heap */
  VirtualPA change_heap_end(long byte_offset);

  VirtualPA get_heap_end() const;

  size_t get_heap_size() const;

  static PhysicalAddress mmu_resolve_va(VirtualAddress va);

 private:
  size_t _custom_index = 0;
  size_t _heap_size = 0;

  PageAllocList _alloc;
  MMUTable* _tbl = nullptr;
};
