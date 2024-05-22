#include <string.h>
#include <sys/file.h>
#include <sys/keyboard.h>
#include <sys/syscall.h>
#include <sys/window.h>

#define INDENT 20
#define TITLE_BAR_HEIGHT 30

static sys_window_t* window = NULL;

static uint32_t current_x, current_y, current_idx;
static uint32_t current_selected = 10;

static sys_bool_t current_is_file = sys_false;
static const char* current_path = NULL;

static void draw_dir(sys_window_t* window, const char* path) {
  sys_dir_t* dir = sys_open_dir(path);

  sys_file_info_t file_info;
  while (SYS_IS_OK(sys_read_dir(dir, &file_info))) {
    if (current_selected == current_idx) {
      sys_gfx_fill_rect(window, current_x - 10, current_y + 5, 5, 5, 0xff6BA4B8);
      current_is_file = !file_info.is_dir;

      static char file_path[256];
      const size_t path_len = strlen(path);
      memcpy(file_path, path, path_len);
      const size_t name_len = strlen(file_info.name);
      memcpy(file_path + path_len, file_info.name, name_len);
      file_path[path_len + name_len] = '\0';

      current_path = file_path;
    } else {
      sys_gfx_draw_rect(window, current_x - 10, current_y + 5, 5, 5, 0xffffffff);
    }

    if (file_info.is_dir) {
      sys_gfx_draw_text(window, current_x, current_y, file_info.name, 0xff0000);

      char sub_path[256];
      const size_t path_len = strlen(path);
      memcpy(sub_path, path, path_len);
      const size_t name_len = strlen(file_info.name);
      memcpy(sub_path + path_len, file_info.name, name_len);
      sub_path[path_len + name_len] = '/';
      sub_path[path_len + name_len + 1] = '\0';

      current_y += 20;
      current_x += INDENT;
      draw_dir(window, sub_path);
      current_x -= INDENT;
    } else {
      sys_gfx_draw_text(window, current_x, current_y, file_info.name, 0xffff00);
      current_y += 20;
    }

    ++current_idx;
  }

  sys_close_dir(dir);
}

static void draw(sys_window_t* window) {
  current_idx = 0;
  current_x = 20;
  current_y = TITLE_BAR_HEIGHT + 20;
  draw_dir(window, "/");
  sys_window_present(window);
}

static void handle_key_event(sys_key_event_t event) {
  if (!sys_is_press_event(event))
    return;

  switch (sys_get_key_code(event)) {
    case SYS_KEY_UP_ARROW:
      if (current_selected == 0)
        return;
      current_selected--;
      draw(window);
      break;
    case SYS_KEY_DOWN_ARROW:
      current_selected++;
      draw(window);
      break;
    case SYS_KEY_ENTER:
      sys_spawn(current_path);
      break;
    default:
      break;
  }
}

int main() {
  sys_print("FILE EXPLORER");

  window = sys_window_create("File Explorer", SYS_POS_DEFAULT, SYS_POS_DEFAULT, 500, 400, SYS_WF_DEFAULT);
  if (window == NULL) {
    sys_print("Failed to create window for file explorer");
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
      case SYS_MSG_KEYDOWN:
        handle_key_event(message.param1);
        break;
      default:
        break;
    }
  }

  sys_window_destroy(window);
  return 0;
}
