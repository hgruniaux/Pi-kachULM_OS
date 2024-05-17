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

static void split_space(MetaPtr block, size_t size) {
  MetaPtr sub_block;
  sub_block = block + size;
  sub_block->is_free = true;
  sub_block->next = block->next;
  sub_block->previous = block;
  sub_block->size = (block->size - size - META_BLOCK_SIZE);
  sub_block->ptr = sub_block->data;
  block->next = sub_block;
}

static MetaPtr extend_heap(MetaPtr last, size_t size, size_t alignment) {  // alignment is always at least 1
  auto* end = (MetaPtr)KernelMemory::get_heap_end();
  const auto heap_offset = size + META_BLOCK_SIZE + alignment;
  auto* brk = (MetaPtr)KernelMemory::change_heap_end(heap_offset);
  if (brk == nullptr) {
    return nullptr;
  }

  end->size = size;
  end->is_free = true;
  end->next = nullptr;
  end->previous = last;
  end->ptr = (void*)(((((uintptr_t)(end->data) - 1) / alignment) * alignment) + alignment);

  if (last != nullptr)
    last->next = end;

  return end;
}

static MetaPtr find_suitable_block(size_t memory, size_t alignment) {
  MetaPtr block = g_malloc_meta_head;
  while (true) {
    const auto offset = (((((uintptr_t)(block->ptr) - 1) / alignment) + 1) * alignment) - (uintptr_t)block->ptr;
    if (block->size >= memory + offset && block->is_free) {
      block->ptr = (MetaPtr)(((((uintptr_t)(block->ptr) - 1) / alignment) + 1) * alignment);
      return block;
    }

    if (block->next == nullptr)
      break;
    block = block->next;
  }

  return extend_heap(block, memory, alignment);
}

static MetaPtr get_block_addr(VirtualPA addr) {
  if (addr < KernelMemory::get_heap_end()) {
    MetaPtr b = g_malloc_meta_head;
    if (b->ptr == (MetaPtr)addr) {
      return b;
    }

    while (b != nullptr && (uintptr_t)b->ptr < addr) {
      b = b->next;
    }

    if (b != nullptr && (uintptr_t)b->ptr + b->size <= addr)
      return nullptr;
    return b;
  }
  return nullptr;
}

static inline bool is_addr_valid(VirtualPA addr) {
  if ((g_malloc_meta_head != nullptr) && (MetaPtr)addr > g_malloc_meta_head && addr < KernelMemory::get_heap_end()) {
    return (addr == (VirtualPA)get_block_addr(addr)->ptr);
  }

  return false;
}

static void merge_block(MetaPtr lhs, MetaPtr rhs) {
  if ((lhs != nullptr) && (rhs != nullptr) && (lhs->next == rhs)) {
    lhs->is_free = lhs->is_free && rhs->is_free;
    lhs->size += rhs->size + META_BLOCK_SIZE;
    lhs->next = rhs->next;
    if (rhs->next != nullptr)
      (rhs->next)->previous = lhs;
  }
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
    block->is_free = false;
    return block->ptr;
  }

  block = find_suitable_block(byte_count, alignment);
  if (block == nullptr) {
    return nullptr;
  }

  block->is_free = false;
  if (block->size > byte_count + META_BLOCK_SIZE) {
    split_space(block, byte_count);
    kfree((block->next)->data);
  }

  return block->ptr;

void kfree(void* ptr) {
  if (ptr == nullptr)
    return;

  if (is_addr_valid((VirtualPA)ptr)) {
    MetaPtr block = get_block_addr((VirtualPA)ptr);
    block->is_free = true;

    if (block->next != nullptr && (block->next)->is_free) {
      merge_block(block, block->next);
    }

    if (block->previous != nullptr && (block->previous)->is_free) {
      merge_block(block->previous, block);
    }
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
