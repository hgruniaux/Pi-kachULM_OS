#include "page_alloc.hpp"
#include <climits>
#include <libk/log.hpp>

uint64_t PageAlloc::memory_needed(uintptr_t nb_pages) {
  return libk::div_round_up(nb_pages * 2, CHAR_BIT);
}

PageAlloc::PageAlloc(size_t nb_pages, void* array) : m_nb_pages(nb_pages), m_mmap(array, memory_needed(nb_pages)) {
  m_mmap.fill_array(true);
  m_mmap.set_bit(0, false);
}

size_t PageAlloc::page_index(PhysicalPA addr) const {
  return m_nb_pages + addr / PAGE_SIZE;
}

bool PageAlloc::page_status(PhysicalPA addr) const {
  return m_mmap.get_bit(page_index(addr));
}

void PageAlloc::mark_as_used(PhysicalPA addr) {
  size_t index = page_index(addr);
  bool side_value = false;
  while (!side_value & (index != 0)) {
    m_mmap.set_bit(index, false);
    side_value = m_mmap.get_bit(index ^ 0x1ul);
    index = index / 2;
  }
}

void PageAlloc::free_page(PhysicalPA addr) {
  size_t index = page_index(addr);
  while (index != 0) {
    if (m_mmap.get_bit(index)) {
      break;
    } else {
      m_mmap.set_bit(index, true);
      index = index / 2;
    }
  }
}

bool PageAlloc::fresh_page(PhysicalPA* addr) {
  size_t index = 1;
  while (true) {
    if (m_mmap.get_bit(index)) {
      if (index >= m_nb_pages) {
        *addr = (index - m_nb_pages) * PAGE_SIZE;
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

/* Test functions

void test_bit_array() {
  uint64_t mon_tableau[2];
  libk::BitArray test = libk::BitArray(mon_tableau, sizeof(mon_tableau) * 8);
  // test.set_bit((size_t)0, true);
  // libk::print("Le premier bit est {}", test.get_bit((size_t)0));
  // libk::print("Le second bit est {}", test.get_bit((size_t)1));
  test.set_bit((size_t)3, false);
  for (size_t i = 0; i <= 65; i++) {
    libk::print("Le bit {} est {}", i, test.get_bit(i));
  }
}

void test_page_alloc() {
  // Test du PageAlloc
  libk::print("Test begin");
  const size_t nb_pages = 64;
  KASSERT(PageAlloc::memory_needed(nb_pages) == 16);
  uint8_t array_mem[16];
  PageAlloc test = PageAlloc(nb_pages, array_mem);
  test.mark_as_used(31);
  libk::print("Mark 1");
  PhysicalPA addr;
  test.fresh_page(&addr);
  libk::print("Mark 2");
  libk::print("Le statut de la page {} est {}", addr, test.page_status(addr));
  while (test.fresh_page(&addr)) {
  }
  test.mmu_free_page(0);
  libk::print("Mark 3");
  libk::print("Le statut de la première page est {}", test.page_status(0));
  if (test.fresh_page(&addr)) {
    libk::print("Test succeeded !");
  } else {
    libk::print("Test failed ...");
  }
}

 */
