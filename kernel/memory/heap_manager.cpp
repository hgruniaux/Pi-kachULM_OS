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

HeapManager::HeapManager(HeapManager::Kind kind, MMUTable* table)
    : _heap_kind(kind),
      _heap_start(kind == Kind::Kernel ? HEAP_MEMORY : PROCESS_HEAP_BASE),
      _heap_va_end(_heap_start),
      _heap_byte_size(0),
      _tbl(table) {}

VirtualAddress HeapManager::change_heap_end(long byte_offset) {
  _heap_byte_size += byte_offset;

  while (_heap_va_end < get_heap_end()) {
    // Increase heap here.
    PhysicalPA new_heap_pa_page = -1;

    if (!memory_impl::get_kernel_alloc()->fresh_page(&new_heap_pa_page)) {
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

    zero_pages(_heap_va_end, 1);
    _heap_va_end += PAGE_SIZE;
  }

  while (_heap_va_end - PAGE_SIZE >= get_heap_end()) {
    // Decrease heap here.

    const VirtualPA va_to_del = _heap_va_end - PAGE_SIZE;
    PhysicalPA pa_to_del;
    zero_pages(va_to_del, 1);

    switch (_heap_kind) {
      case Kind::Process: {
        pa_to_del = _allocated_pa.pop_back();
        break;
      }
      case Kind::Kernel: {
        pa_to_del = memory_impl::resolve_kernel_va(va_to_del, false);
        break;
      }
      default: {
        libk::panic("Unknown heap kind.");
      }
    }

    if (!unmap_range(_tbl, va_to_del, va_to_del)) {
      return 0;
    }

    memory_impl::get_kernel_alloc()->free_page(pa_to_del);

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
        pa_to_del = memory_impl::resolve_kernel_va(va_to_del, false);
        break;
      }
      default: {
        libk::panic("Unknown heap kind.");
      }
    }

    if (!unmap_range(_tbl, va_to_del, va_to_del)) {
      libk::panic("[HeapManager] Unable to free heap.");
    }

    memory_impl::get_kernel_alloc()->free_page(pa_to_del);

    _heap_byte_size -= PAGE_SIZE;
  }
}

VirtualAddress HeapManager::get_heap_start() const {
  return _heap_start;
}
