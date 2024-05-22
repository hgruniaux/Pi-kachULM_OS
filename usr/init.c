#include <sys/syscall.h>

int main() {
  sys_print("INIT");

#define LAUNCH(program)                      \
  do {                                       \
    if (!SYS_IS_OK(sys_spawn(program)))      \
      sys_print("Failed to spawn " program); \
  } while (0)

  // LAUNCH("/usr/bin/credits");
  LAUNCH("/usr/bin/slides");

  while (true)
    asm volatile("");
  return 0;
}
