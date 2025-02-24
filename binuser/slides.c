#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/keyboard.h>
#include <sys/syscall.h>
#include <sys/window.h>

#include "stb_image.h"

static sys_window_t* window = NULL;
static int current_slide = 0;

static bool begin_show = false;

static const uint32_t* slide_pixels = NULL;
static int slide_width = 0, slide_height = 0;

static const uint32_t* next_slide_pixels = NULL;
static int next_slide_width = 0, next_slide_height = 0;

static void draw_welcome_page() {
  sys_gfx_clear(window, 0x000000);
  sys_gfx_draw_text(window, 50, 50, "Press 'B' to begin the show", 0xffffff);
  sys_window_present(window);
}

static void draw_slide() {
  if (!begin_show || slide_pixels == NULL) {
    draw_welcome_page();
    return;
  }

  uint32_t win_width, win_height;
  sys_window_get_geometry(window, NULL, NULL, &win_width, &win_height);

  // The title bar is 30 pixels large.
#define TITLE_BAR_HEIGHT 30

  win_height -= TITLE_BAR_HEIGHT;

  uint32_t x = 0, y = TITLE_BAR_HEIGHT;
  if ((uint32_t)slide_width < win_width)
    x += win_width / 2 - slide_width / 2;
  if ((uint32_t)slide_height < win_height)
    y += win_height / 2 - slide_height / 2;

  sys_gfx_blit(window, x, y, slide_width, slide_height, slide_pixels);
  sys_window_present(window);
}

#define SLIDE_PATH_PREFIX "/slides/"
#define SLIDE_PATH_PREFIX_LEN 8

char* itoa(char* buffer, int res) {
  int size = 0;
  int t = res;

  while (t / 10 != 0) {
    t = t / 10;
    size++;
  }

  size++;
  t = res;
  int i = size - 1;
  while (i >= 0) {
    buffer[i] = (t % 10) + '0';
    t = t / 10;
    i--;
  }

  return buffer + size;
}

static const char* get_slide_path(int idx) {
  static char buffer[512];

  char* it = buffer;
  memcpy(it, SLIDE_PATH_PREFIX, SLIDE_PATH_PREFIX_LEN);
  it = itoa(it + SLIDE_PATH_PREFIX_LEN, idx);
  memcpy(it, ".jpg\0", 5);

  return buffer;
}

static uint8_t* load_slide_image_data(int idx, int* buffer_length) {
  const char* path = get_slide_path(idx);
  sys_print("Reading file...");
  sys_print(path);
  sys_file_t* file = sys_open_file(path, SYS_FM_READ);
  if (file == NULL)
    return NULL;

  const size_t file_size = sys_get_file_size(file);
  uint8_t* buffer = malloc(sizeof(uint8_t) * file_size);
  assert(buffer != NULL);

  size_t read_bytes;
  sys_file_read(file, buffer, file_size, &read_bytes);
  assert(read_bytes == file_size);

  *buffer_length = file_size;

  sys_close_file(file);
  sys_print("Reading done.");

  return buffer;
}

static void convert_to_argb(uint32_t* pixels, int width, int height) {
  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      // The image is in RGBA, we expect ABGR.
      const uint32_t color = __builtin_bswap32(pixels[x + width * y]);
      pixels[x + width * y] = color >> 8;
    }
  }
}

static int load_next_image_slide(int idx) {
  int slide_buffer_len = 0;
  uint8_t* slide_buffer = load_slide_image_data(current_slide, &slide_buffer_len);
  if (slide_buffer == NULL) {
    if (current_slide > 0)
      current_slide--;

    sys_print("Failed to open next slide (invalid file)");
    goto error;
  }

  int width, height;
  uint32_t* new_slide_pixels =
      (uint32_t*)stbi_load_from_memory(slide_buffer, slide_buffer_len, &width, &height, NULL, 4);
  free((void*)slide_buffer);

  if (new_slide_pixels == NULL) {
    if (current_slide > 0)
      current_slide--;


    sys_print("Failed to open next slide (invalid image)");
    goto error;
  }

  free((void*)slide_pixels);

  sys_print("Converting to ARGB");
  convert_to_argb(new_slide_pixels, width, height);
  sys_print("Done. Ready to draw slide.");

  next_slide_pixels = new_slide_pixels;
  next_slide_width = width;
  next_slide_height = height;
  return 1;

error:
  next_slide_pixels = NULL;
  next_slide_width = 0;
  next_slide_height = 0;
  return 0;
}

static void update_current_slide() {
  if (next_slide_pixels != NULL) {
    free((void*)slide_pixels);
    slide_pixels = next_slide_pixels;
    slide_width = next_slide_width;
    slide_height = next_slide_height;
    draw_slide();
  }

  if (!load_next_image_slide(current_slide)) {
    current_slide--;
  }
}

static void handle_key_event(sys_key_event_t event) {
  if (!sys_is_press_event(event))
    return;

  switch (sys_get_key_code(event)) {
    case SYS_KEY_B: {
      if (!begin_show) {
        begin_show = true;
        current_slide++;
        update_current_slide();
      }
      break;
    }
    case SYS_KEY_LEFT_ARROW:
      if (current_slide == 0)
        return;
      current_slide--;
      update_current_slide();
      break;
    case SYS_KEY_SPACE:
    case SYS_KEY_RIGHT_ARROW:
      current_slide++;
      update_current_slide();
      break;
    case SYS_KEY_C: {
      if (!sys_is_alt_pressed(event))
        break;

      if (!SYS_IS_OK(sys_spawn("/bin/credits")))
        sys_print("Failed to spawn credits :/");

      break;
    }

    default:
      break;
  }
}

int main() {
  sys_print("SLIDES");

  window = sys_window_create("Slides", SYS_POS_CENTERED, SYS_POS_CENTERED, 1231 + 20, 724 + 20, SYS_WF_DEFAULT);
  if (window == NULL) {
    sys_print("Failed to create window for slides");
    return 1;
  }

  if (begin_show) {
    load_next_image_slide(current_slide);
    update_current_slide();
  } else {
    draw_welcome_page();
    load_next_image_slide(current_slide);
  }

  bool should_close = false;
  while (!should_close) {
    sys_message_t message;
    sys_wait_message(window, &message);
    switch (message.id) {
      case SYS_MSG_CLOSE:
        should_close = true;
        break;
      case SYS_MSG_RESIZE:
        draw_slide();
        break;
      case SYS_MSG_KEYDOWN:
        handle_key_event(message.param1);
        break;
      default:
        break;
    }
  }

  if (slide_pixels != NULL)
    free((void*)slide_pixels);

  sys_window_destroy(window);
  return 0;
}
