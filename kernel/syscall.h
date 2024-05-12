#ifndef PIKAOS_SYSCALL_H
#define PIKAOS_SYSCALL_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif  // !__ASSEMBLER__

// The system call identifiers:
#define SYS_EXIT 1
#define SYS_SLEEP 2
#define SYS_YIELD 3
#define SYS_PRINT 4
#define SYS_SPAWN 5
#define SYS_DEBUG 100

#ifndef __ASSEMBLER__
typedef int sys_error_t;
#endif  // !__ASSEMBLER__
// The system call error codes:
#define SYS_ERR_OK 0
#define SYS_ERR_INTERNAL 1

#ifndef __ASSEMBLER__
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void sys_exit(int64_t __status) __attribute__((__noreturn__));
sys_error_t sys_sleep(uint64_t __time_in_s);
sys_error_t sys_usleep(uint64_t __time_in_us);
sys_error_t sys_print(const char* __msg);
sys_error_t sys_spawn(void (*__f)());
sys_error_t sys_yield();
sys_error_t sys_debug();

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // !__ASSEMBLER__

#endif  // !PIKAOS_SYSCALL_H
