#include "process_memory.hpp"

ProcessMemory::ProcessMemory() {}

bool ProcessMemory::map_page(size_t target_address, MemoryPage page) {
  (void)target_address;
  (void)page;
  (void)tbl;
  return false;
}

void ProcessMemory::activate() {}
