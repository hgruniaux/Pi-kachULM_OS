#include "mem_alloc.hpp"
#include "libk/log.hpp"
#include "memory.hpp"

#include <libk/utils.hpp>

using MetaPtr = struct MetaBlock*;

struct MetaBlock {
  size_t size;
  MetaPtr next;
  MetaPtr previous;
  void* ptr;
  bool is_free;
  char data[1];
};  // struct MetaBlock

constexpr size_t META_BLOCK_SIZE = offsetof(MetaBlock, data);

static MetaPtr g_malloc_meta_head = nullptr;

static void split_space(MetaPtr block, size_t bytes_count) {
  KASSERT(block != nullptr);
  MetaPtr sub_block;
  uintptr_t next_addr = (uintptr_t)block->ptr + bytes_count;

  KASSERT(next_addr < KernelMemory::get_heap_end());

  sub_block = (MetaPtr)next_addr;
  sub_block->is_free = true;
  sub_block->next = block->next;
  sub_block->previous = block;
  sub_block->size = block->size - bytes_count - META_BLOCK_SIZE;
  block->size = bytes_count;
  sub_block->ptr = sub_block->data;
  block->next = sub_block;
}

static MetaPtr extend_heap(MetaPtr last, size_t size, size_t alignment) {  // alignment is always at least 1
  auto end = (MetaPtr)KernelMemory::get_heap_end();
  uintptr_t next_addr = libk::align_to_next((uintptr_t)end + META_BLOCK_SIZE, alignment);
  auto heap_offset = next_addr - (uintptr_t)end + size;
  auto brk = KernelMemory::change_heap_end(heap_offset);
  if (brk == 0) {
    return nullptr;
  }

  end->size = size;
  end->is_free = false;
  end->next = nullptr;
  end->previous = last;
  end->ptr = (void*)next_addr;

  if (last != nullptr)
    last->next = end;

  return end;
}

static MetaPtr find_suitable_block(size_t memory, size_t alignment) {
  MetaPtr block = g_malloc_meta_head;
  while (block != nullptr && block->next != nullptr) {
    const size_t offset = libk::align_to_next((uintptr_t)block->ptr, alignment) - (uintptr_t)block->ptr;
    if (block->size >= memory + offset && block->is_free) {
      auto next_addr = (uintptr_t)block->ptr + offset;
      block->ptr = (void*)next_addr;
      block->is_free = false;
      block->size -= offset;
      return block;
    }

    block = block->next;
  }

  return extend_heap(block, memory, alignment);
}

static MetaPtr get_block_addr(VirtualAddress addr) {
  if (addr < KernelMemory::get_heap_end()) {
    MetaPtr b = g_malloc_meta_head;
    if (b != nullptr && (VirtualAddress)b->ptr == addr) {
      return b;
    }

    // LOG_INFO("Getting block for addr {}", addr);
    // LOG_INFO("Heap end at addr {}", KernelMemory::get_heap_end());

    while (b != nullptr && (uintptr_t)b->ptr + b->size < addr) {
      // if (b->next==nullptr)
      // {
      //   LOG_INFO("Last block infos = {}, {}", (VirtualAddress)b->ptr, b->size);
      // }

      b = b->next;
    }

    // if (b==nullptr)
    // {
    //   LOG_INFO("Not matching block found");
    // }

    if (b != nullptr && (uintptr_t)b->ptr > addr)
      return nullptr;
    return b;
  }
  return nullptr;
}

static inline bool is_addr_valid(VirtualAddress addr) {
  if (g_malloc_meta_head == nullptr)
    return false;

  if (addr > (VirtualAddress)g_malloc_meta_head && addr < KernelMemory::get_heap_end()) {
    MetaPtr b = get_block_addr(addr);
    return (b != nullptr && addr == (VirtualAddress)b->ptr);
  }

  // LOG_INFO("Unbounded addr {}", addr);
  // LOG_INFO("Heap end at addr {}", KernelMemory::get_heap_end());
  return false;
}

static void merge_block(MetaPtr lhs, MetaPtr rhs) {
  KASSERT(lhs != nullptr);
  KASSERT(rhs != nullptr);
  KASSERT(lhs->next == rhs);
  KASSERT(rhs->is_free);

  lhs->size += rhs->size + META_BLOCK_SIZE;
  lhs->next = rhs->next;
  if (rhs->next != nullptr)
    (rhs->next)->previous = lhs;
}

void* kmalloc(size_t byte_count, size_t alignment) {
  if (alignment == 0)
    alignment++;

  if (byte_count == 0)
    byte_count++;  // ensure that we have a unique pointer address even when allocating 0 bytes

  MetaPtr block;
  if (g_malloc_meta_head == nullptr) {
    block = extend_heap(nullptr, byte_count, alignment);
    if (block == nullptr) {
      return nullptr;
    }
    g_malloc_meta_head = block;
    return block->ptr;
  }

  block = find_suitable_block(byte_count, alignment);
  if (block == nullptr) {
    return nullptr;
  }

#if 0
  if (block->size > byte_count + META_BLOCK_SIZE) {
    split_space(block, byte_count);
    // LOG_INFO("Free addr {}", (VirtualAddress)(block->next)->ptr);
    kfree((block->next)->ptr);
  }
#endif

  return block->ptr;
}

void kfree(void* ptr) {
  if (ptr == nullptr)
    return;

  // if(!is_addr_valid((VirtualAddress)ptr)) {
  //   LOG_INFO("Unvalid addr {}",(VirtualAddress)ptr); }

  KASSERT(is_addr_valid((VirtualAddress)ptr));

  MetaPtr block = get_block_addr((VirtualAddress)ptr);
  block->is_free = true;
  block->ptr = block->data;
  block->size += (uintptr_t)ptr - (uintptr_t)block->ptr;

  if (block->next != nullptr && (block->next)->is_free) {
    // LOG_INFO("Merging to right");
    merge_block(block, block->next);
  }

  if (block->previous != nullptr && (block->previous)->is_free) {
    // LOG_INFO("Merging to left");
    if (!((block->previous)->next == block)) {
      if (get_block_addr((VirtualAddress)((block->previous)->next)->ptr) == block) {
        (block->previous)->next = block;
        LOG_WARNING("Décalage adresse block");
        merge_block(block->previous, block);
      } else {
        LOG_WARNING("Block de merde");
      }
      return;
    }

    merge_block(block->previous, block);
  }
}

#include <new>

// Provide the C++ operators new and delete:

void* operator new(size_t bytes_count) {
  return kmalloc(bytes_count, alignof(std::max_align_t));
}

void* operator new(size_t bytes_count, std::align_val_t alignment) {
  return kmalloc(bytes_count, (size_t)alignment);
}

void* operator new[](size_t bytes_count) {
  return kmalloc(bytes_count, alignof(std::max_align_t));
}

void* operator new[](size_t bytes_count, std::align_val_t alignment) {
  return kmalloc(bytes_count, (size_t)alignment);
}

void operator delete(void* ptr) {
  kfree(ptr);
}

void operator delete(void* ptr, size_t) {
  kfree(ptr);
}

void operator delete[](void* ptr) {
  kfree(ptr);
}

void operator delete[](void* ptr, size_t) {
  kfree(ptr);
}
