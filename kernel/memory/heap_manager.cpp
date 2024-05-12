#include "memory/heap_manager.hpp"
#include <libk/log.hpp>
#include "boot/mmu_utils.hpp"
#include "kernel_internal_memory.hpp"

static inline constexpr PagesAttributes kernel_rw_memory = {.sh = Shareability::InnerShareable,
                                                            .exec = ExecutionPermission::NeverExecute,
                                                            .rw = ReadWritePermission::ReadWrite,
                                                            .access = Accessibility::Privileged,
                                                            .type = MemoryType::Normal};

static inline constexpr PagesAttributes process_rw_memory = {.sh = Shareability::InnerShareable,
                                                             .exec = ExecutionPermission::NeverExecute,
                                                             .rw = ReadWritePermission::ReadWrite,
                                                             .access = Accessibility::AllProcess,
                                                             .type = MemoryType::Normal};

static PageAllocList* _alloc = nullptr;

HeapManager::HeapManager(HeapManager::Kind kind, MMUTable* table)
    : _heap_kind(kind),
      _heap_start(kind == Kind::Kernel ? HEAP_MEMORY : PROCESS_HEAP_BASE),
      _heap_va_end(_heap_start),
      _heap_byte_size(0),
      _tbl(table) {
  if (_alloc == nullptr) {
    _alloc = memory_impl::get_kernel_alloc();
  }
}

VirtualAddress HeapManager::change_heap_end(long byte_offset) {
  _heap_byte_size += byte_offset;

  while (_heap_va_end < get_heap_end()) {
    // Increase heap here.
    PhysicalPA new_heap_pa_page = -1;

    if (!_alloc->fresh_page(&new_heap_pa_page)) {
      return 0;
    }

    switch (_heap_kind) {
      case Kind::Process: {
        if (!map_range(_tbl, _heap_va_end, _heap_va_end, new_heap_pa_page, process_rw_memory)) {
          return 0;
        }

        _allocated_pa.push_back(new_heap_pa_page);
        break;
      }
      case Kind::Kernel: {
        if (!map_range(_tbl, _heap_va_end, _heap_va_end, new_heap_pa_page, kernel_rw_memory)) {
          return 0;
        }
        break;
      }
    }

    _heap_va_end += PAGE_SIZE;
  }

  while (_heap_va_end - PAGE_SIZE >= get_heap_end()) {
    // Decrease heap here.

    const VirtualPA va_to_del = _heap_va_end - PAGE_SIZE;
    PhysicalPA pa_to_del;

    switch (_heap_kind) {
      case Kind::Process: {
        pa_to_del = _allocated_pa.pop_back();
        break;
      }
      case Kind::Kernel: {
        pa_to_del = resolve_kernel_va(va_to_del);
        break;
      }
    }

    if (!unmap_range(_tbl, va_to_del, va_to_del)) {
      return 0;
    }

    _alloc->free_page(pa_to_del);

    _heap_va_end -= PAGE_SIZE;
  }

  return get_heap_end();
}

VirtualAddress HeapManager::get_heap_end() const {
  return _heap_start + get_heap_byte_size();
}

size_t HeapManager::get_heap_byte_size() const {
  return _heap_byte_size;
}

PhysicalPA HeapManager::resolve_kernel_va(VirtualAddress va) {
  asm volatile("at s1e1w, %x0" ::"r"(va));

  uint64_t par_el1;
  asm volatile("mrs %x0, PAR_EL1" : "=r"(par_el1));

  if (par_el1 & 0x1) {
    libk::panic("[HeapManager] Unable to resolve physical address from the Kernel heap.");
  }

  return par_el1 & libk::mask_bits(12, 47);
}

void HeapManager::free() {
  while (_heap_byte_size > 0) {
    const VirtualPA va_to_del = get_heap_end() - PAGE_SIZE;
    PhysicalPA pa_to_del;

    switch (_heap_kind) {
      case Kind::Process: {
        pa_to_del = _allocated_pa.pop_back();
        break;
      }
      case Kind::Kernel: {
        pa_to_del = resolve_kernel_va(va_to_del);
        break;
      }
    }

    if (!unmap_range(_tbl, va_to_del, va_to_del)) {
      libk::panic("[HeapManager] Unable to free heap.");
    }

    _alloc->free_page(pa_to_del);

    _heap_byte_size -= PAGE_SIZE;
  }
}

VirtualAddress HeapManager::get_heap_start() const {
  return _heap_start;
}
