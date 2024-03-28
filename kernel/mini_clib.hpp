#pragma once

#include <cstddef>

#undef strlen
#undef strcmp
#undef strncmp
#undef strrchr
#undef strchrnul

size_t strlen(const char* str);
int strcmp(const char* lhs, const char* rhs);
int strncmp(const char* lhs, const char* rhs, size_t count);
char* strrchr(const char* str, int ch);
char* strchrnul(const char* str, int ch);
