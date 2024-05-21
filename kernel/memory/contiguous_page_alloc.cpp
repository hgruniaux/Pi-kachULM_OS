#include "contiguous_page_alloc.hpp"
#include <climits>
#include <libk/utils.hpp>
#include "boot/mmu_utils.hpp"
#include "libk/log.hpp"

uint64_t ContiguousPageAllocator::memory_needed(size_t nb_pages) {
  return libk::div_round_up(nb_pages, CHAR_BIT);  // We only use one bit per page.
}

ContiguousPageAllocator::ContiguousPageAllocator(size_t nb_pages, uintptr_t array)
    : _nb_pages(nb_pages), _free_page(array, memory_needed(nb_pages)), _page_cursor(0) {
  _free_page.fill_array(true);
}

size_t ContiguousPageAllocator::page_index(PhysicalPA addr) const {
  return addr / PAGE_SIZE;
}

bool ContiguousPageAllocator::try_fresh_pages(size_t nb_pages, PhysicalPA* addr, bool first_pass) {
  PhysicalPA page_candidate = -1;
  size_t found_free_pages = 0;

  for (; page_index(_page_cursor) < _nb_pages; _page_cursor += PAGE_SIZE) {
    if (_free_page[page_index(_page_cursor)]) {
      if (found_free_pages == 0) {
        // Found a new candidate for our chunk.
        page_candidate = _page_cursor;
        found_free_pages = 1;
      } else {
        // One more contiguous page found !
        found_free_pages++;
      }

      if (found_free_pages == nb_pages) {
        KASSERT(page_candidate != -1ull);
        mark_as_used(page_candidate, nb_pages);
        *addr = page_candidate;
        return true;
      }
    } else {
      // This page is not free, so we revert our search variables
      page_candidate = -1ull;
      found_free_pages = 0;
    }
  }

  if (first_pass) {
    // We have searched for all the pages. Let's go back from the beginning.
    _page_cursor = 0;
    return try_fresh_pages(nb_pages, addr, false);
  } else {
    // We made a second pass, and still no page found :/
    return false;
  }
}

void ContiguousPageAllocator::mark_as_used(PhysicalPA addr, size_t nb_pages) {
  if (page_index(addr) + nb_pages >= _nb_pages) {
    LOG_ERROR("Page index {} is not in range. Cannot be marked as used.", page_index(addr) + nb_pages);
    libk::panic("Marking page as used failed.");
  }

  for (size_t page_i = page_index(addr); page_i <= page_index(addr) + nb_pages; ++page_i) {
    _free_page.set_bit(page_i, false);
  }
}

bool ContiguousPageAllocator::fresh_pages(size_t nb_pages, PhysicalPA* addr) {
  return try_fresh_pages(nb_pages, addr, true);
}

void ContiguousPageAllocator::free_pages(PhysicalPA addr, size_t nb_pages) {
  if (page_index(addr) + nb_pages >= _nb_pages) {
    LOG_ERROR("Page index {} is not in range. Cannot be marked as free.", page_index(addr) + nb_pages);
    libk::panic("Marking page as freed failed.");
  }

  for (size_t page_i = page_index(addr); page_i <= page_index(addr) + nb_pages; ++page_i) {
    _free_page.set_bit(page_i, true);
  }
}

bool ContiguousPageAllocator::page_status(PhysicalPA addr) const {
  if (page_index(addr) < _nb_pages) {
    return _free_page[page_index(addr)];
  } else {
    return false;
  }
}
