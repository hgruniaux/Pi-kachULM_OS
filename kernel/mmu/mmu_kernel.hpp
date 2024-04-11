#pragma once
#include <cstdint>
#include "libk/bit_array.hpp"
#define PAGESIZE 4096

typedef void* physical_address_t;

/*
      Page Allocator
*/

class PageAlloc {
 public:
  // Construcor
  PageAlloc(uint64_t memsize);
  // Utility functions
  void setmmap(void* array);
  void mark_as_used(physical_address_t addr);
  bool freshpage(physical_address_t* addr);
  void freepage(physical_address_t addr);
  bool page_status(physical_address_t addr);
  // Constant
  const uint64_t memory_needed;

 protected:
  libk::BitArray m_mmap;
  const uint64_t m_memsize;
  const uint64_t m_pagequant;
};

/*
      Memory Allocator
*/

class Malloc : public PageAlloc {
  public : 
    // Constructor
    Malloc(PageAlloc ancestor);
    // Utility
    void* malloc(size_t nb_oct, uint64_t align);
    void free(void* memory_allocated);

  private :
    PageAlloc m_ancestor;

};