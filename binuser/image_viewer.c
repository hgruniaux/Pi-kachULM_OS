#include <sys/file.h>
#include <sys/syscall.h>
#include <sys/window.h>
#include <assert.h>

#include "stb_image.h"

static uint8_t* read_file(const char* path, int* buffer_length) {
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
  sys_print("Done.");

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

int main() {
  size_t argc = sys_get_argc();
  const char** argv = sys_get_argv();

  if (argc <= 1 || argc > 2) {
    sys_print("ERROR: bad arguments for image viewer.");
    return 1;
  }

  const char* image_path = argv[1];
  int image_length = 0;
  const uint8_t* image_data = read_file(image_path, &image_length);
  if (image_data == NULL) {
    sys_print("ERROR: failed to read image file.");
    return 1;
  }

  int width, height;
  const uint32_t* image_pixels =
      (const uint32_t*)stbi_load_from_memory(image_data, image_length, &width, &height, NULL, 4);
  free(image_data);
  if (image_pixels == NULL) {
    sys_print("ERROR: failed to load image data (maybe not a valid image).");
    return 1;
  }

  sys_window_t* window =
      sys_window_create("Image viewer", SYS_POS_DEFAULT, SYS_POS_DEFAULT, width, height, SYS_WF_DEFAULT);
  if (window == NULL) {
    free(image_pixels);
    sys_print("ERROR: failed to create window.");
    return 1;
  }

  convert_to_argb(image_pixels, width, height);
  sys_gfx_blit(window, 0, 0, width, height, image_pixels);
  sys_window_present(window);

  bool should_close = false;
  while (!should_close) {
    sys_message_t message;
    sys_wait_message(window, &message);
    switch (message.id) {
      case SYS_MSG_CLOSE:
        should_close = true;
        break;
      default:
        break;
    }
  }

  sys_window_destroy(window);
  return 0;
}
