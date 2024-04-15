#include "mmu_kernel.hpp"
#include <cstddef>
#include <cstdint>
#include "libk/string.hpp"
#include "libk/log.hpp"
#include "libk/utils.hpp"

/*
      Page Allocator
*/

PageAlloc::PageAlloc(uint64_t memsize)
    : memory_needed((memsize*2) / PAGESIZE), m_memsize(memsize), m_pagequant(memsize / PAGESIZE), m_mmap(nullptr, 0) {}

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
  while (!side_value & (index != 0)) {
    m_mmap.set_bit(index, false);
    side_value = m_mmap.get_bit(index^0x1ul);
    // libk::print("Index = {} , Side_value = {} , func = {}", index, side_value, index^0x1ul);
    index = index / 2;
  }
}

void PageAlloc::freepage(physical_address_t addr) {
  size_t index = m_pagequant + (uint64_t)addr / PAGESIZE;
  while (index != 0) {
    if (m_mmap.get_bit(index)) {
      break;
    } else {
      m_mmap.set_bit(index, true);
      index = index / 2;
    }
  }
}

bool PageAlloc::freshpage(physical_address_t* addr) {
  size_t index = 1;
  while (true) {
    if (m_mmap.get_bit(index)) {
      if (index >= (size_t)m_pagequant) {
        *addr = (physical_address_t)((index - m_pagequant) * PAGESIZE);
        libk::print("La page alouée est {}, son index est {}", (index - m_pagequant), index);
        mark_as_used(*addr);
        return true;
      } else {
        index = index * 2;
      }
    } else if (index % 2) {
      return false;
    } else {
      index += 1;
    }
  }
}

/*
      Memory Allocator
*/
