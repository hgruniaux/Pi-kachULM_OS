#include <stddef.h>
#include <syscall/message.h>
#include <syscall/syscall.h>

int main();

void _start() {
  int exit_code = main();
  sys_exit(exit_code);
}

int main() {
  sys_pid_t pid = sys_getpid();
  if (pid == 0) {
    while (1)
      continue;
  }

  while (1) {
    sys_sleep(pid * 2);
    sys_debug(pid);
  }

  return 0;
}
