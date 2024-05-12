#include <syscall/syscall.h>

int main();

void _start() {
  int exit_code = main();
  sys_exit(exit_code);
}

int main() {
  sys_debug();
  sys_yield();
  sys_debug();
  return 0;
}
