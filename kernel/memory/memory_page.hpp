#pragma once
#include "mmu_table.hpp"

class MemoryPageBuilder;

class MemoryPage {
 public:
  MemoryPage() = default;

  [[nodiscard]] size_t write(size_t byte_offset, const void* data, size_t data_byte_length) const;
  [[nodiscard]] size_t read(size_t byte_offset, void* data, size_t data_byte_length) const;

  void free();
 private:
  friend MemoryPageBuilder;
  explicit MemoryPage(PhysicalPA pa, VirtualPA va, MemoryPageBuilder* builder);

  PhysicalPA _pa;
  VirtualPA _kernel_va;
  MemoryPageBuilder* _builder;
};
