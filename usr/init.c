#include <sys/syscall.h>

int main() {
  sys_print("INIT");

#define LAUNCH(program)               \
  if (!SYS_IS_OK(sys_spawn(program))) \
  sys_print("Failed to spawn " program)

  LAUNCH("/usr/bin/credits");

  while (true)
    asm volatile("");
  return 0;
}
