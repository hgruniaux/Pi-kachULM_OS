#include <stddef.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/window.h>

int main();

void _start() {
  int exit_code = main();
  sys_exit(exit_code);
}

int main() {
  sys_pid_t pid = sys_getpid();
  srand(pid);

  int n = 50;
  while (n-- > 0)
    rand();

  sys_debug(pid);
  if (pid == 0) {
    while (true)
      sys_yield();
  }

  const uint32_t width = 500;
  const uint32_t height = 400;
  int32_t x = 40 + (pid - 2) * 300;
  int32_t y = 40 + pid * 40;
  sys_debug(x);
  sys_debug(y);

  char title[] = "Window 0";
  title[7] = '0' + pid;
  sys_window_t* window = sys_window_create(title, x, y, width, height, SYS_WF_DEFAULT);
  if (window == NULL)
    return 1;

  uint32_t color;
  static uint32_t colors[] = {0x5695EA, 0x174C0A, 0xDA9213, 0xFA2D72};

  if (pid > 1 && (pid - 2) <= (sizeof(colors) / sizeof(colors[0])))
    color = colors[pid - 2];
  else
    color = 0;

  sys_gfx_clear(window, color);
  sys_window_set_visibility(window, true);

  bool should_close = false;
  while (!should_close) {
    sys_message_t message;
    while (sys_poll_message(window, &message)) {
      switch (message.id) {
        case SYS_MSG_CLOSE:
          should_close = true;
          break;
        case SYS_MSG_RESIZE:
          sys_gfx_clear(window, color);
          break;
        default:
          break;
      }
    }

    sys_usleep(500000);
  }

  sys_window_destroy(window);
  return 0;
}
