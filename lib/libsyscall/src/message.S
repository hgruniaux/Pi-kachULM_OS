#include <syscall/syscall.h>

// sys_error_t sys_poll_msg(sys_msg_t*);
.global sys_poll_msg
sys_poll_msg:
  mov w8, #SYS_POLL_MSG
  svc #0
  ret

// sys_error_t sys_wait_msg(sys_msg_t*);
.global sys_wait_msg
sys_wait_msg:
  mov w8, #SYS_WAIT_MSG
  svc #0
  ret
