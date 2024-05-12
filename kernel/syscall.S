#include "syscall.h"

// void __syscall0(uint32_t __id);
.global __syscall0 __syscall0 : mov w8,
    w0 svc #0 ret

        // void __syscall1(uint32_t __id, uint64_t __arg0);
        .global __syscall1 __syscall1 : mov w8,
    w0 mov x0,
    x1 svc #0 ret

        .global process3 process3 : mov w8,
    #SYS_DEBUG svc #0 mov w8, #SYS_EXIT mov x0,
    #0 svc #0

        .global process1 process1 : sub x28,
    x28, #16 mov w0, #5 str w0, [ x28, 12 ] mov w8, #SYS_DEBUG svc #0 mov w8, #SYS_SPAWN adr x0, process3 svc #0 mov w8,
    #SYS_YIELD svc #0 mov w8, #SYS_DEBUG svc #0 mov w8, #SYS_EXIT mov x0, #0 svc #0
