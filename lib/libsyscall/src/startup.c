#include <sys/syscall.h>

extern int main();

// The program entry point. Call the standard C entry point: main().
void _start() {
  int exit_code = main();
  sys_exit(exit_code);
}
