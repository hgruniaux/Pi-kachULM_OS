#ifndef __PIKAOS_LIBC_ASSERT_H__
#define __PIKAOS_LIBC_ASSERT_H__

#include "sys/__types.h"
#include "sys/__utils.h"

#ifndef __STRINGIFY
#define __STRINGIFY_X(x) #x
#define __STRINGIFY(x) __STRINGIFY_X(x)
#endif  // !__STRINGIFY

#ifndef NDEBUG
#define assert(cond) \
  (void)((cond) || (__assert_failed(__FILE__ ":" __STRINGIFY(__LINE__) ": assertion failed: " #cond), 0))
#else
#define assert(cond)
#endif  // !NDEBUG

__SYS_EXTERN_C_BEGIN

__attribute__((__noreturn__)) void __assert_failed(const char*);

__SYS_EXTERN_C_END

#endif  // !__PIKAOS_LIBC_ASSERT_H__
