#include <syscall/syscall.h>

// void __syscall0(uint32_t __id);
.global __syscall0
__syscall0:
    mov w8, w0
    svc #0
    ret

// void __syscall1(uint32_t __id, uint64_t __arg0);
.global __syscall1
__syscall1:
  mov w8, w0
  mov x0, x1
  svc #0
  ret
