#include <string.h>
#include <sys/file.h>
#include <sys/keyboard.h>
#include <sys/syscall.h>
#include <sys/window.h>

#define INDENT 25
#define TITLE_BAR_HEIGHT 30
#define PADDING 10

#define ICON_SIZE 16
#define ICON_PADDING 5
#define LINE_HEIGHT 20

static sys_window_t* window = NULL;

static uint32_t current_x, current_y, current_idx;
static uint32_t current_selected = 1;

static sys_bool_t current_is_file = sys_false;
static char current_path_buffer[256] = { 0 };
static const char* current_path = NULL;

#define OPEN_STATE_HASHTABLE_SIZE 1024
static bool open_state_hashtable[OPEN_STATE_HASHTABLE_SIZE] = { false };

static int hash(const char* path) {
  int hash = 0;
  for (size_t i = 0; i < strlen(path); ++i) {
    if (path[i] == '/')
      continue; // ignore slash to avoid problems with paths like /path/ and /path wich are equivalent
    hash = (hash * 31) + path[i];
  }
  return hash % OPEN_STATE_HASHTABLE_SIZE;
}

static bool is_folder_open(const char* path) {
  const int hash_value = hash(path);
  return open_state_hashtable[hash_value];
}

static void toggle_folder_open(const char* path) {
  const int hash_value = hash(path);
  if (open_state_hashtable[hash_value])
    open_state_hashtable[hash_value] = false;
  else
    open_state_hashtable[hash_value] = true;
}

static void draw_dir(sys_window_t* window, const char* path) {
  sys_dir_t* dir = sys_open_dir(path);

  sys_file_info_t file_info;
  while (SYS_IS_OK(sys_read_dir(dir, &file_info))) {
    const int icon_x = current_x;
    if (current_selected == current_idx) {
      sys_gfx_fill_rect(window, current_x, current_y + (LINE_HEIGHT / 2) - (ICON_SIZE / 2), ICON_SIZE, ICON_SIZE, 0xff6BA4B8);
      current_is_file = !file_info.is_dir;

      const size_t path_len = strlen(path);
      memcpy(current_path_buffer, path, path_len);
      const size_t name_len = strlen(file_info.name);
      memcpy(current_path_buffer + path_len, file_info.name, name_len);
      current_path_buffer[path_len + name_len] = '\0';

      current_path = current_path_buffer;
    } else {
      sys_gfx_draw_rect(window, current_x, current_y + (LINE_HEIGHT / 2) - (ICON_SIZE / 2), ICON_SIZE, ICON_SIZE, 0xffffffff);
    }

    ++current_idx;

    const int text_x = current_x + ICON_SIZE + ICON_PADDING;
    const int text_y = current_y;
    if (file_info.is_dir) {
      sys_gfx_draw_text(window, text_x, text_y, file_info.name, 0xff0000);

      char sub_path[256];
      const size_t path_len = strlen(path);
      memcpy(sub_path, path, path_len);
      const size_t name_len = strlen(file_info.name);
      memcpy(sub_path + path_len, file_info.name, name_len);
      sub_path[path_len + name_len] = '/';
      sub_path[path_len + name_len + 1] = '\0';

      const bool is_open = is_folder_open(sub_path);

      current_y += LINE_HEIGHT;
      if (is_open) {
        current_x += INDENT;
        draw_dir(window, sub_path);
        current_x -= INDENT;
      }
    } else {
      sys_gfx_draw_text(window, text_x, text_y, file_info.name, 0xffff00);
      current_y += LINE_HEIGHT;
    }
  }

  sys_close_dir(dir);
}

static void draw(sys_window_t* window) {
  sys_gfx_clear(window, 0xff000000);
  current_idx = 0;
  current_x = PADDING + 10;
  current_y = TITLE_BAR_HEIGHT + PADDING;
  draw_dir(window, "/");
  sys_window_present(window);
}

static bool ends_with(const char* text, const char* pattern) {
  const size_t text_len = strlen(text);
  const size_t pattern_len = strlen(pattern);
  if (text_len < pattern_len)
    return false;

  return memcmp(text + text_len - pattern_len, pattern, pattern_len) == 0;
}

static void open_file(const char* path) {
  const bool is_image = ends_with(path, ".png") || ends_with(path, ".jpg") || ends_with(path, ".jpeg");
  if (is_image) {
    if (!SYS_IS_OK(sys_spawn2("/bin/image_viewer", 2, (const char*[]){"/bin/image_viewer", path})))
      sys_print("Failed to spawn the image viewer");
    return;
  }

  if (SYS_IS_OK(sys_spawn(current_path)))
    return;

  if (!SYS_IS_OK(sys_spawn2("/bin/text_viewer", 2, (const char*[]){"/bin/text_viewer", path})))
    sys_print("Failed to spawn the text viewer");
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
      if (current_is_file) {
        open_file(current_path);
      } else {
        toggle_folder_open(current_path);
        draw(window);
      }
      break;
    default:
      break;
  }
}

int main() {
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
