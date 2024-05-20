#pragma once

#include <cstddef>
#include <cstdint>
#include "hardware/dma/dma_controller.hpp"
#include "memory.hpp"

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
  const size_t nb_pages;
  const PhysicalPA buffer_pa;
  const VirtualPA kernel_va;
};
