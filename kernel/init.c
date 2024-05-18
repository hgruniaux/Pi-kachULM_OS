#include <stddef.h>
#include <sys/syscall.h>
#include <sys/window.h>

int main();

void _start() {
  int exit_code = main();
  sys_exit(exit_code);
}

uint32_t pitch;
uint32_t* framebuffer;

int main() {
  sys_pid_t pid = sys_getpid();
  sys_debug(pid);
  if (pid == 0) {
    while (true)
      sys_yield();
  }

  const uint32_t width = 600;
  const uint32_t height = 400;
  int32_t x = 50;
  int32_t y = 50;

  sys_window_t* window = sys_window_create("My Window", x, y, width, 400, SYS_WF_DEFAULT);
  if (window == NULL)
    return 1;

  sys_window_set_visibility(window, true);
  framebuffer = sys_window_get_framebuffer(window, &pitch);

  uint32_t color;
  static uint32_t colors[] = {0x5695EA, 0x174C0A, 0xDA9213, 0xFA2D72};

  if (pid > 0 && pid <= (sizeof(colors) / sizeof(colors[0])))
    color = colors[pid - 1];
  else
    color = 0;

  for (uint32_t x = 0; x < width; ++x) {
    for (uint32_t y = 0; y < height; ++y) {
      framebuffer[x + pitch * y] = color;
    }
  }

  bool should_close = false;
  sys_word_t i = 10;
  while (!should_close) {
    x += pid;
    y += pid * 2;
    sys_window_set_geometry(window, x, y, width, height);

    sys_message_t message;
    while (sys_poll_message(window, &message)) {
      switch (message.id) {
        case SYS_MSG_CLOSE:
          should_close = true;
          break;
        case SYS_MSG_RESIZE:
          // Framebuffer is reallocated when the window is resized.
          framebuffer = sys_window_get_framebuffer(window, &pitch);
          break;
        case SYS_MSG_REPAINT:
          sys_window_present(window);
          break;
        default:
          break;
      }
    }

    sys_debug(i++);
    sys_window_present(window);
    sys_yield();
  }

  sys_window_destroy(window);
  return 0;
}
