#ifndef __PIKAOS_LIBC_STDLIB_H__
#define __PIKAOS_LIBC_STDLIB_H__

#include "sys/__types.h"
#include "sys/__utils.h"

__SYS_EXTERN_C_BEGIN

void* malloc(size_t __n);
void free(void*);

__SYS_EXTERN_C_END

#endif  // !__PIKAOS_LIBC_STDLIB_H__
