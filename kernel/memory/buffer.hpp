#pragma once

#include <cstddef>
#include <cstdint>
#include "hardware/dma/dma_controller.hpp"
#include "libk/linked_list.hpp"
#include "memory.hpp"

class ProcessMemory;

class Buffer {
 public:
  /** Creates a memory buffer of @a byte_size bytes. */
  Buffer(uint32_t byte_size);

  /** Free this buffer. */
  ~Buffer();

  /** Returns the number of bytes of this buffer. */
  [[nodiscard]] size_t get_byte_size() const;

  /** Returns the raw pointer of this buffer.
   * Reading or Writing before or after the buffer's end is undefined. */
  [[nodiscard]] void* get() const;

  /** Returns the DMA Address of this buffer. */
  [[nodiscard]] DMA::Address get_dma_address();

 private:
//  const size_t nb_pages;
  PhysicalPA buffer_pa_start;
  PhysicalPA buffer_pa_end;
  VirtualPA kernel_va;

  friend ProcessMemory;

  struct ProcessMapped {
    ProcessMapped(VirtualPA start, ProcessMemory* proc) : buffer_start(start), proc(proc) {}
    VirtualPA buffer_start;
    ProcessMemory* proc;
  };

  libk::LinkedList<ProcessMapped> _proc;

  void register_mapping(ProcessMemory* proc_mem, VirtualPA start_addr);
  void unregister_mapping(ProcessMemory* proc_mem);
  VirtualPA end_address(VirtualPA start_address);
};
