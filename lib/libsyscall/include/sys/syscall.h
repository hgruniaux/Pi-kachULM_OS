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

#define SYS_PID_CURRENT 0

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
  SYS_ERR_INVALID_FILE,
  SYS_ERR_INVALID_DIR,
  SYS_ERR_INVALID_PID,
  SYS_ERR_INVALID_PIPE,
  SYS_ERR_PIPE_FULL,
  SYS_ERR_PIPE_EMPTY,
  SYS_ERR_PIPE_CLOSED,
};

#define SYS_IS_OK(e) ((e) == SYS_ERR_OK)

__SYS_EXTERN_C_BEGIN

void sys_exit(int64_t status) __attribute__((__noreturn__));
sys_error_t sys_sleep(uint64_t time_in_s);
sys_error_t sys_usleep(uint64_t time_in_us);
sys_error_t sys_print(const char* msg);
sys_error_t sys_spawn(const char* path);
sys_error_t sys_spawn2(const char* path, size_t argc, const char** argv);
sys_error_t sys_yield();
sys_pid_t sys_getpid();
sys_error_t sys_sched_set_priority(sys_pid_t pid, uint32_t priority);
sys_error_t sys_sched_get_priority(sys_pid_t pid, uint32_t* priority);
sys_error_t sys_debug(uint64_t x);
sys_error_t sys_get_framebuffer(void** pixels, uint32_t* width, uint32_t* height, uint32_t* stride);

/*
 * Pipe API
 */

#define SYS_PIPE_NAME_MAX 32
#define SYS_PIPE_STDIN "stdin"
#define SYS_PIPE_STDOUT "stdout"
#define SYS_PIPE_STDERR "stderr"
typedef struct sys_pipe_t sys_pipe_t;
sys_pipe_t* sys_pipe_open(const char* name);
sys_error_t sys_pipe_close(sys_pipe_t* pipe);
sys_pipe_t* sys_pipe_get(sys_pid_t pid, const char* name);
sys_error_t sys_pipe_read(sys_pipe_t* pipe, void* buffer, size_t size, size_t* read_bytes);
sys_error_t sys_pipe_write(sys_pipe_t* pipe, const void* buffer, size_t size, size_t* written_bytes);

/*
 * Program arguments API
 */

#define SYS_ARGS_ADDRESS 0x1000000
size_t sys_get_argc();
const char** sys_get_argv();

/*
 * Heap API
 */

void* sys_sbrk(ptrdiff_t increment);

__SYS_EXTERN_C_END

#endif  // !PIKAOS_LIBC_SYS_SYSCALL_H
