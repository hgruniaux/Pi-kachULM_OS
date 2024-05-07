#pragma once

#include <cstddef>
#include <cstdint>

class MemoryPageBuilder {
 public:
  bool create_memory_page(MemoryPage* page);

  void unregister_page(PhysicalPA physical_addr, VirtualPA kernel_virtual_addr);

 private:
  size_t _custom_page_cpt = 0;
};
