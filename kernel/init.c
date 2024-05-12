#include <stddef.h>
#include <syscall/syscall.h>

int main();

void _start() {
  int exit_code = main();
  sys_exit(exit_code);
}

static const char* hex_digits = "0123456789ABCDEF";

void pp_uint(uint64_t value) {
  char buffer[29] = "0x";

  for (size_t i = 0; i < sizeof(uint64_t) * 2; ++i) {
    buffer[i + 2] = hex_digits[(value >> (4 * (sizeof(uint64_t) * 2 - (i + 1)))) & 0xf];
  }

  buffer[28] = 0;
  sys_print(buffer);
}

void pp_stack() {
  uint64_t sp;
  asm volatile("mov %x0, sp" : "=r"(sp));
  pp_uint(sp);
}

int main() {
  sys_debug();
  sys_yield();
  sys_debug();
  sys_print("Hello from process !");
  sys_print("My stack is:");
  pp_stack();
  sys_print("END !");
  return 0;
}
