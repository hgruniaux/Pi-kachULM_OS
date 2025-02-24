#include <sys/syscall.h>

size_t sys_get_argc() {
  return *(const uint64_t*)(SYS_ARGS_ADDRESS);
}

const char** sys_get_argv() {
  return (const char**)(SYS_ARGS_ADDRESS + sizeof(uint64_t));
}

