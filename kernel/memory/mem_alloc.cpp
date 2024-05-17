#include "mem_alloc.hpp"
#include "libk/log.hpp"
#include "memory.hpp"

#include <libk/utils.hpp>

void* kmalloc(size_t byte_count, size_t alignment) {
  if (alignment == 0)
    alignment++;

  if (byte_count == 0)
    byte_count++;  // ensure that we have a unique pointer address even when allocating 0 bytes

  const uintptr_t heap_end = KernelMemory::get_heap_end();
  const uintptr_t target_ptr = libk::align_to_next(heap_end, alignment);
  const long offset = target_ptr - heap_end + byte_count;
  const uintptr_t new_end = KernelMemory::change_heap_end(offset);

  LOG_INFO("We need {} aligned on {}", byte_count, alignment);
  LOG_INFO("Heap End: {:#x}", heap_end);
  LOG_INFO("Target Pointer: {:#x}", target_ptr);
  LOG_INFO("Offset: {}", offset);
  LOG_INFO("New Heap End: {:#x}", new_end);

  libk::bzero((void*)heap_end, new_end - heap_end);

  return (void*)target_ptr;
}

void kfree(void* ptr) {
  if (ptr == nullptr)
    return;
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
