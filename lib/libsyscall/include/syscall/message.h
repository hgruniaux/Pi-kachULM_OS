#ifndef PIKAOS_MESSAGE_H
#define PIKAOS_MESSAGE_H

#ifdef __ASSEMBLER__
#error This file can not be included from assembly.
#endif  // __ASSEMBLER__

#include <stdint.h>
#include "syscall.h"

// The system message identifiers:
#define SYS_MSG_NULL 0
#define SYS_MSG_KEYDOWN 1
#define SYS_MSG_KEYUP 2

typedef struct __sys_msg_t {
  uint32_t id;
  uint64_t timestamp;
} sys_msg_t;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

sys_error_t sys_poll_msg(sys_msg_t* __msg);
sys_error_t sys_wait_msg(sys_msg_t* __msg);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // !PIKAOS_MESSAGE_H
