#include <sys/syscall.h>

void sys_exit(int64_t status) {
  __syscall1(SYS_EXIT, (uint64_t)status);
  __builtin_unreachable();
}

sys_error_t sys_sleep(uint64_t time_in_s) {
  const uint64_t time_in_us = time_in_s * 1000000;
  return sys_usleep(time_in_us);
}

sys_error_t sys_usleep(uint64_t time_in_us) {
  return __syscall1(SYS_SLEEP, time_in_us);
}

sys_error_t sys_print(const char* msg) {
  return __syscall1(SYS_PRINT, (sys_word_t)msg);
}

sys_error_t sys_yield() {
  return __syscall0(SYS_YIELD);
}

sys_pid_t sys_getpid() {
  return __syscall0(SYS_GETPID);
}

sys_error_t sys_sched_set_priority(sys_pid_t pid, uint32_t priority) {
  return __syscall2(SYS_SCHED_SET_PRIORITY, pid, priority);
}

sys_error_t sys_sched_get_priority(sys_pid_t pid, uint32_t* priority) {
  return __syscall2(SYS_SCHED_SET_PRIORITY, pid, (sys_word_t)priority);
}

sys_error_t sys_debug(uint64_t x) {
  return __syscall1(SYS_DEBUG, x);
}

void* sys_sbrk(ptrdiff_t __increment) {
  return (void*)__syscall1(SYS_SBRK, __increment);
}
