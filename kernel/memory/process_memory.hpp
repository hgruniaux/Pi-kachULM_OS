#pragma once

#include <libk/linked_list.hpp>

#include "buffer.hpp"
#include "memory/heap_manager.hpp"
#include "memory_chunk.hpp"

class ProcessMemory {
 public:
  explicit ProcessMemory(size_t minimum_stack_byte_size);
  ~ProcessMemory();

  /** Returns the ASID (Address Space ID) for this process. */
  uint8_t get_asid() const;

  /* Stack Management */
  VirtualAddress get_stack_end() const;
  VirtualAddress get_stack_start() const;

  /* Heap Management */
  VirtualPA change_heap_end(long byte_offset);
  VirtualPA get_heap_end() const;
  size_t get_heap_byte_size() const;

  /** Change the memory mapping to take this process memory in account. */
  void activate() const;

  /** Change the memory mapping to remove any process memory in account. */
  static void deactivate();

  /* Memory chunk management */
  bool map_chunk(MemoryChunk& chunk, VirtualPA address, bool read_only, bool executable);
  bool map_buffer(Buffer& chunk, VirtualPA address, bool read_only, bool executable);

  void unmap_memory(VirtualPA start_address);
  bool change_memory_attr(VirtualPA start_address, bool read_only, bool executable);

  void free();

  bool is_read_only(VirtualPA va) const;
  bool is_executable(VirtualPA va) const;

 private:
  static uint8_t _new_asid;
  MMUTable _tbl;

  HeapManager _heap;
  MemoryChunk _stack;

  struct MappedSections {
    MappedSections(VirtualPA start, bool is_buffer, void* mem) : start(start), is_buffer(is_buffer), mem(mem) {}
    VirtualPA start;
    bool is_buffer;
    void* mem;
  };

  libk::LinkedList<MappedSections> _sec;
};
