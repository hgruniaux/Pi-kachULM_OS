#include <stdlib.h>
#include <sys/syscall.h>

void __assert_failed(const char* msg) {
  sys_print(msg);
  sys_exit(1);
}
