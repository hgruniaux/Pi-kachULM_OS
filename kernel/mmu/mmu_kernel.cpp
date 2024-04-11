#include "mmu_kernel.hpp"
#include <cstddef>
#include <cstdint>
#include "libk/string.hpp"

/*
      Page Allocator
*/

PageAlloc::PageAlloc(uint64_t memsize)
    : m_memsize(memsize), m_pagequant(memsize / PAGESIZE), memory_needed(m_pagequant / 4), m_mmap(nullptr, 0) {}

void PageAlloc::setmmap(void* array) {
  m_mmap = libk::BitArray(array, memory_needed);
}

bool PageAlloc::page_status(physical_address_t addr) {
  size_t index = m_pagequant + (uint64_t)addr / PAGESIZE;
  return m_mmap.get_bit(index);
}

void PageAlloc::mark_as_used(physical_address_t addr) {
  size_t index = m_pagequant + (uint64_t)addr / PAGESIZE;
  bool side_value = false;
  while (~side_value) {
    m_mmap.set_bit(index, false);
    side_value = m_mmap.get_bit((index / (2 * sizeof(uint8_t))) << sizeof(uint8_t) + ~(index % sizeof(uint8_t)));
    // index = place_to_index(index_to_place(index) >> sizeof(uint8_t));
    index = index >> sizeof(uint8_t);
  }
}

void PageAlloc::freepage(physical_address_t addr) {
  size_t index = m_pagequant + (uint64_t)addr / PAGESIZE;
  while (index != 0) {
    if (m_mmap.get_bit(index)) {
      index = 0;
    } else {
      m_mmap.set_bit(index, true);
      // index = place_to_index(index_to_place(index) >> sizeof(uint8_t));
      index = index >> sizeof(uint8_t);
    }
  }
}

bool PageAlloc::freshpage(physical_address_t* addr) {
  size_t index = 0;
  while (true) {
    if (m_mmap.get_bit(index)) {
      if (index >= (size_t)m_pagequant) {
        addr = (physical_address_t*)((index - m_pagequant) * PAGESIZE);
        return true;
      } else {
        index = index << sizeof(uint8_t);
      }
    } else if (index % sizeof(uint64_t)) {
      return false;
    } else {
      index += 1;
    }
  }
}

/*
      Memory Allocator
*/
