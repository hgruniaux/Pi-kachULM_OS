#pragma once

#include "mmu_table.hpp"
#include "memory_page.hpp"

class ProcessMemory {
 public:
  ProcessMemory();

  bool map_page(size_t target_address, MemoryPage page);

  void activate();
 private:
  MMUTable tbl;
};
