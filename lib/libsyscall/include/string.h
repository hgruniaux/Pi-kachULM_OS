#ifndef __PIKAOS_LIBC_STRING_H__
#define __PIKAOS_LIBC_STRING_H__

#include "sys/__types.h"
#include "sys/__utils.h"

__SYS_EXTERN_C_BEGIN

size_t strlen(const char*);

int memcmp(const void* __lhs, const void* __rhs, size_t __length);
void* memset(void* __dst, int __val, size_t __length);
void* memcpy(void* __dst, const void* __src, size_t __length);
void* memmove(void* __dst, const void* __src, size_t __length);

__SYS_EXTERN_C_END

#endif  // !__PIKAOS_LIBC_STRING_H__
