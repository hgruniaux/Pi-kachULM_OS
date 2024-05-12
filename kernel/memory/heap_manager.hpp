#pragma once

#include <cstddef>
#include <cstdint>
#include <libk/linked_list.hpp>

#include "memory/memory.hpp"
#include "memory/page_alloc_list.hpp"

class HeapManager {
 public:
  enum class Kind { Process, Kernel };
  HeapManager() = default;
  HeapManager(Kind kind, MMUTable* table);

  VirtualPA change_heap_end(long byte_offset);

  VirtualPA get_heap_end() const;

  size_t get_heap_size() const;

  void free();
 private:
  Kind _heap_kind;

  uintptr_t _heap_start = 0;
  size_t _heap_size = 0;
  MMUTable* _tbl = nullptr;

  libk::LinkedList<PhysicalPA> _allocated_pa;

  static PhysicalPA resolve_kernel_va(VirtualAddress va);
};
