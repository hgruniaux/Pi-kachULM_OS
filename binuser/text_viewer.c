#include <sys/file.h>
#include <sys/syscall.h>
#include <sys/window.h>
#include <assert.h>

#include "stb_image.h"

static const uint8_t* text_data = NULL;
static int text_length = 0;

static uint8_t* read_file(const char* path, int* buffer_length) {
  sys_print("Reading file...");
  sys_print(path);
  sys_file_t* file = sys_open_file(path, SYS_FM_READ);
  if (file == NULL)
    return NULL;

  const size_t file_size = sys_get_file_size(file);
  uint8_t* buffer = malloc(sizeof(uint8_t) * (file_size + 1));
  assert(buffer != NULL);

  size_t read_bytes;
  sys_file_read(file, buffer, file_size, &read_bytes);
  assert(read_bytes == file_size);

  buffer[file_size] = '\0';
  *buffer_length = file_size;

  sys_close_file(file);
  sys_print("Done.");

  return buffer;
}

static void draw(sys_window_t* window) {
  sys_gfx_draw_text(window, 10, 40, text_data, 0xffffffff);
  sys_window_present(window);
}

int main() {
  size_t argc = sys_get_argc();
  const char** argv = sys_get_argv();

  if (argc <= 1 || argc > 2) {
    sys_print("ERROR: bad arguments for text viewer.");
    return 1;
  }

  const char* text_path = argv[1];
  text_data = read_file(text_path, &text_length);
  if (text_data == NULL) {
    sys_print("ERROR: failed to read file.");
    return 1;
  }

  sys_window_t* window =
      sys_window_create("Text viewer", SYS_POS_DEFAULT, SYS_POS_DEFAULT, 600, 400, SYS_WF_DEFAULT);
  if (window == NULL) {
    free(text_data);
    sys_print("ERROR: failed to create window.");
    return 1;
  }

  draw(window);

  bool should_close = false;
  while (!should_close) {
    sys_message_t message;
    sys_wait_message(window, &message);
    switch (message.id) {
      case SYS_MSG_CLOSE:
        should_close = true;
        break;
      case SYS_MSG_RESIZE:
        draw(window);
        break;
      default:
        break;
    }
  }

  sys_window_destroy(window);
  return 0;
}
