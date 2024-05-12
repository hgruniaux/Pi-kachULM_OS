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

  VirtualAddress change_heap_end(long byte_offset);

  VirtualAddress get_heap_end() const;

  VirtualAddress get_heap_start() const;

  size_t get_heap_byte_size() const;

  void free();

 private:
  Kind _heap_kind;

  VirtualAddress _heap_start = 0;
  VirtualAddress _heap_va_end = 0;
  size_t _heap_byte_size = 0;
  MMUTable* _tbl = nullptr;

  libk::LinkedList<PhysicalPA> _allocated_pa;

  static PhysicalAddress resolve_kernel_va(VirtualAddress va);
};
