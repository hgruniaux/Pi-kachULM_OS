#include "mem_alloc.hpp"

/*
      Memory Allocator
*/

namespace Malloc {

meta_ptr base = nullptr;  // init at 0

meta_ptr find_suitable_block(size_t memory, uint64_t align) {
  meta_ptr b = base;
  meta_ptr end = (meta_ptr)KernelMemory::get_heap_end();
  if (b) {
    while (b->ptr < end) {
      size_t offset = (((((uintptr_t)(b->ptr) - 1) / align) + 1) * align) - (uintptr_t)b->ptr;
      if (b->size >= memory + offset) {
        b->ptr = (meta_ptr)(((((uintptr_t)b - 1) / align) + 1) * align);
        return b;
      }
    }
  }
  return extend_heap(b, memory, align);
}

void split_space(meta_ptr block, size_t size) {
  meta_ptr sub_block;
  sub_block = block + size;
  sub_block->is_free = 1;
  sub_block->next = block->next;
  sub_block->previous = block;
  sub_block->size = (block->size - size - META_BLOCK_SIZE);
  sub_block->ptr = sub_block->data;
  block->next = sub_block;
}

meta_ptr extend_heap(meta_ptr last, size_t size, uint64_t align) {  // align is always at least 1
  meta_ptr end, brk;
  end = (meta_ptr)KernelMemory::get_heap_end();
  long heap_offset = (size + META_BLOCK_SIZE + align) / PAGESIZE;
  brk = (meta_ptr)KernelMemory::change_heap_end(heap_offset);
  if (brk == nullptr) {
    return nullptr;
  } else {
    end->size = size;
    end->is_free = 1;
    end->next = nullptr;
    end->previous = last;
    *(end->data) = (((((uintptr_t)(end->data) - 1) / align) * align) + align);
    end->ptr = end->data;
    if (last) {
      last->next = end;
    }
    return end;
  }
}

bool is_addr_valid(VirtualPA addr) {
  if (base && (meta_ptr)addr > base && addr < KernelMemory::get_heap_end()) {
    return (addr = (VirtualPA)get_block_addr(addr)->ptr);
  }
  return 0;
}

meta_ptr get_block_addr(VirtualPA addr) {
  if (addr < KernelMemory::get_heap_end()) {
    meta_ptr b = base;
    while (b->ptr <= (meta_ptr)addr) {
      b = b->next;
    }
    return b->previous;
  }
  return nullptr;
}

void merge_block(meta_ptr block1, meta_ptr block2) {
  if (block1 && block2 && (block1->next = block2)) {
    block1->is_free = block1->is_free && block2->is_free;
    block1->size += block2->size + META_BLOCK_SIZE;
    block1->next = block2->next;
    (block2->next)->previous = block1;
  }
}

VirtualPA malloc(size_t nb_octet, uint64_t align) {
  if (!align) {
    align++;
  }
  meta_ptr block;
  if (!base) {
    block = extend_heap(0, nb_octet, align);
    if (!block) {
      return 0;
    } else {
      base = block;
      block->is_free = 0;
      return (VirtualPA)block->data;
    }
  } else {
    block = find_suitable_block(nb_octet, align);
    if (!block) {
      return 0;
    } else {
      block->is_free = 0;
      if (block->size > nb_octet + META_BLOCK_SIZE) {
        split_space(block, nb_octet);
        free(*(block->next)->data);
      }
      return (VirtualPA)block->data;
    }
  }
}

void free(VirtualPA memory_allocated) {
  if (is_addr_valid(memory_allocated)) {
    meta_ptr block = get_block_addr(memory_allocated);
    block->is_free = 1;
    if ((block->next)->is_free) {
      merge_block(block, block->next);
    }
    if ((block->previous)->is_free) {
      merge_block(block->previous, block);
    }
  }
}

}  // namespace Malloc


/*
        Test function
*/

// void test_malloc() {
//     VirtualPA addr_test = Malloc::malloc(64, 0);
//     libk::print("Check 1");
//     libk::print("L'addresse attribuée est {}", addr_test);
//     libk::print("Les bornes du heap sont {} et {}", KernelMemory::get_heap_start(), KernelMemory::get_heap_end());
//     VirtualPA addr2_test = Malloc::malloc(2048,64);
//     libk::print("L'adresse 2 est {}", addr2_test);
//     Malloc::free(addr_test);
//     libk::print("check 2");
//     VirtualPA addr3_test = Malloc::malloc(64, 0);
//     libk::print("La nouvelle addresse est {}", );
// }