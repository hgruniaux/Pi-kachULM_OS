#pragma once

#include <cstddef>

extern "C" __attribute__((malloc)) void* kmalloc(size_t byte_count, size_t alignment);
extern "C" void kfree(void* ptr);
extern "C" void* krealloc(void* ptr, size_t new_size);
