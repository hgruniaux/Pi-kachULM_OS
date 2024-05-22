#include <assert.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/syscall.h>
#include <sys/window.h>

static void draw_slide(sys_window_t* window) {}

static void cat_file(const char* path) {
  sys_file_t* file = sys_open_file(path, SYS_FM_READ);
  if (file == NULL) {
    sys_print("Failed to open file");
    return;
  }

  size_t file_size = sys_get_file_size(file);
  char* buffer = malloc(sizeof(char) * file_size);
  assert(buffer != NULL);

  size_t read_bytes;
  sys_file_read(file, buffer, file_size, &read_bytes);
  assert(read_bytes == file_size);

  sys_print(buffer);

  sys_close_file(file);
}

int main() {
  sys_print("SLIDES");

  cat_file("/keep.me");

  sys_window_t* window = sys_window_create("Slides", SYS_POS_CENTERED, SYS_POS_CENTERED, 800, 600, SYS_WF_DEFAULT);
  if (window == NULL) {
    sys_print("Failed to create window for credits");
    return 1;
  }

  draw_slide(window);

  bool should_close = false;
  while (!should_close) {
    sys_message_t message;
    sys_wait_message(window, &message);
    switch (message.id) {
      case SYS_MSG_CLOSE:
        should_close = true;
        break;
      case SYS_MSG_RESIZE:
        draw_slide(window);
        break;
      default:
        break;
    }
  }

  sys_window_destroy(window);
  return 0;
}
