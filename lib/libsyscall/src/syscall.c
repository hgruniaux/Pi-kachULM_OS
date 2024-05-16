#include <syscall/syscall.h>

// Defined in syscall.S.
sys_error_t __syscall0(uint32_t __id);
sys_error_t __syscall1(uint32_t __id, uint64_t __arg0);

void sys_exit(int64_t __status) {
  __syscall1(SYS_EXIT, (uint64_t)__status);
  __builtin_unreachable();
}

sys_error_t sys_sleep(uint64_t __time_in_s) {
  const uint64_t __time_in_us = __time_in_s * 1000000;
  return sys_usleep(__time_in_us);
}

sys_error_t sys_usleep(uint64_t __time_in_us) {
  return __syscall1(SYS_SLEEP, __time_in_us);
}

sys_error_t sys_print(const char* __msg) {
  return __syscall1(SYS_PRINT, (uint64_t)(uintptr_t)__msg);
}

sys_error_t sys_spawn(void (*__f)()) {
  return __syscall1(SYS_SPAWN, (uint64_t)(uintptr_t)__f);
}

sys_error_t sys_yield() {
  return __syscall0(SYS_YIELD);
}
