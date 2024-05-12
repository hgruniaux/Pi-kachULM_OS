#pragma once
#include "memory/mmu_table.hpp"

#include <libk/linked_list.hpp>

class ProcessMemory;

class MemoryChunk {
 public:
  MemoryChunk(size_t nb_pages);
  ~MemoryChunk();

  [[nodiscard]] size_t write(size_t byte_offset, const void* data, size_t data_byte_length);
  [[nodiscard]] size_t read(size_t byte_offset, void* data, size_t data_byte_length) const;

  void free();

  size_t byte_size() const;
  bool is_status_okay() const;

 private:
  friend ProcessMemory;

  const size_t _nb_pages;
  PhysicalPA* _pas;

  VirtualPA _kernel_va;

  struct ProcessMapped {
    VirtualPA chunk_start;
    ProcessMemory* mem;
  };

  libk::LinkedList<ProcessMapped> _proc;

  void register_mapping(ProcessMemory* proc_mem, VirtualPA start_addr);
  void unregister_mapping(ProcessMemory* proc_mem);
  VirtualPA end_address(VirtualPA start_address);
};
