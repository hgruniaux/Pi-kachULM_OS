#ifndef __PIKAOS_LIBC_SYS___TYPES_H__
#define __PIKAOS_LIBC_SYS___TYPES_H__

#include <stddef.h>
#include <stdint.h>

typedef int sys_error_t;
typedef uint32_t sys_pid_t;
typedef uint64_t sys_word_t;

#ifdef __cplusplus
typedef bool sys_bool_t;
#define sys_true true
#define sys_false false
#elif __has_include(<stdbool.h>)
#include <stdbool.h>
typedef bool sys_bool_t;
#define sys_true true
#define sys_false false
#else
typedef uint8_t sys_bool_t;
#define sys_true 1
#define sys_false 0
#endif  // __cplusplus

#endif  // !__PIKAOS_LIBC_SYS___TYPES_H__
