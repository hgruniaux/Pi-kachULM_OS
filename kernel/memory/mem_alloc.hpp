#pragma once
#include <cstdint>
#include "libk/bit_array.hpp"
#include "memory.hpp"
#define PAGESIZE 4096
#define META_BLOCK_SIZE 21

/*
      Memory Allocator
*/
namespace Malloc {

typedef struct meta_block* meta_ptr;

struct meta_block {
  size_t size;
  char is_free;
  meta_ptr next;
  meta_ptr previous;
  void* ptr;
  char data[1];
};

// Interface
VirtualPA malloc(size_t nb_octet, uint64_t align);
void free(VirtualPA memory_allocated);

// Utility
meta_ptr find_suitable_block(size_t memory, uint64_t align);
void split_space(meta_ptr block, size_t nb_octet);
meta_ptr extend_heap(meta_ptr last, size_t nb_octet, uint64_t align);
bool is_addr_valid(VirtualPA addr);
meta_ptr get_block_addr(VirtualPA addr);
void merge_block(meta_ptr block1, meta_ptr block2);

};  // namespace Malloc
