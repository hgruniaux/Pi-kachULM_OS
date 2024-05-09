#pragma once

#include <cstddef>

extern "C" void* kmalloc(size_t byte_count, size_t alignment);
extern "C" void kfree(void* ptr);
