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
#define SYS_GETPID 6
#define SYS_SCHED_SET_PRIORITY 7
#define SYS_SCHED_GET_PRIORITY 8
#define SYS_DEBUG 50
#define SYS_GFX_DRAW_LINE 100
#define SYS_GFX_DRAW_RECT 101
#define SYS_GFX_FILL_RECT 102
#define SYS_GFX_DRAW_TEXT 103

#ifndef __ASSEMBLER__
typedef int sys_error_t;
typedef uint32_t sys_pid_t;
#endif  // !__ASSEMBLER__
// The system call error codes:
#define SYS_ERR_OK 0
#define SYS_ERR_INTERNAL 1
#define SYS_ERR_UNKNOWN_SYSCALL 2
#define SYS_ERR_INVALID_PRIORITY 3

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
sys_pid_t sys_getpid();
sys_error_t sys_sched_set_priority(sys_pid_t __pid, uint32_t __priority);
sys_error_t sys_sched_get_priority(sys_pid_t __pid, uint32_t* __priority);
sys_error_t sys_debug(uint64_t __x);

sys_error_t sys_gfx_draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t argb);
sys_error_t sys_gfx_draw_rect(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t argb);
sys_error_t sys_gfx_fill_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t argb);
sys_error_t sys_gfx_draw_text(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t argb);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // !__ASSEMBLER__

#endif  // !PIKAOS_SYSCALL_H
