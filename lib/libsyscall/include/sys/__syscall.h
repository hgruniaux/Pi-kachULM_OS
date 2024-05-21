#ifndef __PIKAOS_LIBC_SYS___SYSCALL_H__
#define __PIKAOS_LIBC_SYS___SYSCALL_H__

#include "__types.h"
#include "__utils.h"

__SYS_EXTERN_C_BEGIN

static inline __attribute__((unused)) sys_word_t __syscall0(uint32_t __id) {
  register uint32_t __id_reg asm("w8") = __id;
  register sys_word_t __ret asm("x0");
  asm volatile("svc #0" : "=r"(__ret) : "r"(__id_reg));
  return __ret;
}

static inline __attribute__((unused)) sys_word_t __syscall1(uint32_t __id, sys_word_t __x0) {
  register uint32_t __id_reg asm("w8") = __id;
  register sys_word_t __x0_reg asm("x0") = __x0;
  register sys_word_t __ret asm("x0");
  asm volatile("svc #0" : "=r"(__ret) : "r"(__id_reg), "r"(__x0_reg));
  return __ret;
}

static inline __attribute__((unused)) sys_word_t __syscall2(uint32_t __id, sys_word_t __x0, sys_word_t __x1) {
  register uint32_t __id_reg asm("w8") = __id;
  register sys_word_t __x0_reg asm("x0") = __x0;
  register sys_word_t __x1_reg asm("x1") = __x1;
  register sys_word_t __ret asm("x0");
  asm volatile("svc #0" : "=r"(__ret) : "r"(__id_reg), "r"(__x0_reg), "r"(__x1_reg));
  return __ret;
}

static inline __attribute__((unused)) sys_word_t __syscall3(uint32_t __id,
                                                            sys_word_t __x0,
                                                            sys_word_t __x1,
                                                            sys_word_t __x2) {
  register uint32_t __id_reg asm("w8") = __id;
  register sys_word_t __x0_reg asm("x0") = __x0;
  register sys_word_t __x1_reg asm("x1") = __x1;
  register sys_word_t __x2_reg asm("x2") = __x2;
  register sys_word_t __ret asm("x0");
  asm volatile("svc #0" : "=r"(__ret) : "r"(__id_reg), "r"(__x0_reg), "r"(__x1_reg), "r"(__x2_reg));
  return __ret;
}

static inline __attribute__((unused)) sys_word_t
__syscall4(uint32_t __id, sys_word_t __x0, sys_word_t __x1, sys_word_t __x2, sys_word_t __x3) {
  register uint32_t __id_reg asm("w8") = __id;
  register sys_word_t __x0_reg asm("x0") = __x0;
  register sys_word_t __x1_reg asm("x1") = __x1;
  register sys_word_t __x2_reg asm("x2") = __x2;
  register sys_word_t __x3_reg asm("x3") = __x3;
  register sys_word_t __ret asm("x0");
  asm volatile("svc #0" : "=r"(__ret) : "r"(__id_reg), "r"(__x0_reg), "r"(__x1_reg), "r"(__x2_reg), "r"(__x3_reg));
  return __ret;
}

static inline __attribute__((unused)) sys_word_t
__syscall5(uint32_t __id, sys_word_t __x0, sys_word_t __x1, sys_word_t __x2, sys_word_t __x3, sys_word_t __x4) {
  register uint32_t __id_reg asm("w8") = __id;
  register sys_word_t __x0_reg asm("x0") = __x0;
  register sys_word_t __x1_reg asm("x1") = __x1;
  register sys_word_t __x2_reg asm("x2") = __x2;
  register sys_word_t __x3_reg asm("x3") = __x3;
  register sys_word_t __x4_reg asm("x4") = __x4;
  register sys_word_t __ret asm("x0");
  asm volatile("svc #0"
               : "=r"(__ret)
               : "r"(__id_reg), "r"(__x0_reg), "r"(__x1_reg), "r"(__x2_reg), "r"(__x3_reg), "r"(__x4_reg));
  return __ret;
}

__SYS_EXTERN_C_END

#endif  // !__PIKAOS_LIBC_SYS___SYSCALL_H__
