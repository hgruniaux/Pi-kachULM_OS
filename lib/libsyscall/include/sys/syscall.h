#ifndef PIKAOS_LIBC_SYS_SYSCALL_H
#define PIKAOS_LIBC_SYS_SYSCALL_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif  // !__ASSEMBLER__

#include "__syscall.h"
#include "syscall_table.h"

#ifndef __ASSEMBLER__
typedef int sys_error_t;
typedef uint32_t sys_pid_t;
typedef uint64_t sys_word_t;
#endif  // !__ASSEMBLER__
// The system call error codes:

enum {
  SYS_ERR_OK,
  SYS_ERR_GENERIC,
  SYS_ERR_INTERNAL,
  SYS_ERR_UNKNOWN_SYSCALL,
  SYS_ERR_INVALID_PRIORITY,
  SYS_ERR_MSG_QUEUE_EMPTY,
  SYS_ERR_OUT_OF_MEM,
  SYS_ERR_INVALID_WINDOW,
  SYS_ERR_INVALID_FILE
};

#define SYS_IS_OK(e) ((e) == SYS_ERR_OK)

__SYS_EXTERN_C_BEGIN

void sys_exit(int64_t status) __attribute__((__noreturn__));
sys_error_t sys_sleep(uint64_t time_in_s);
sys_error_t sys_usleep(uint64_t time_in_us);
sys_error_t sys_print(const char* msg);
sys_error_t sys_spawn(const char* path);
sys_error_t sys_yield();
sys_pid_t sys_getpid();
sys_error_t sys_sched_set_priority(sys_pid_t pid, uint32_t priority);
sys_error_t sys_sched_get_priority(sys_pid_t pid, uint32_t* priority);
sys_error_t sys_debug(uint64_t x);

void* sys_sbrk(ptrdiff_t increment);

__SYS_EXTERN_C_END

#endif  // !PIKAOS_LIBC_SYS_SYSCALL_H
