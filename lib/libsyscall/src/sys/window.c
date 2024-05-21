#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/window.h>

struct __sys_window_t {
  // Internal handle to identify the window inside the kernel.
  sys_word_t kernel_handle;
  uint32_t flags;

  // The saved window title (UTF-8 encoded).
  char* title;
};  // struct __sys_window_t

sys_window_t* sys_window_create(const char* title,
                                int32_t x,
                                int32_t y,
                                uint32_t width,
                                uint32_t height,
                                uint32_t flags) {
  assert(title != NULL);

  sys_window_t* window = (sys_window_t*)malloc(sizeof(sys_window_t));
  if (window == NULL)
    goto error;

  memset(window, 0, sizeof(sys_window_t));
  window->flags = flags;

  window->kernel_handle = __syscall1(SYS_WINDOW_CREATE, flags);
  if (window->kernel_handle == 0)
    goto error;  // kernel_handle is 0 only in case of error.

  // Configure the window.
  if (!SYS_IS_OK(sys_window_set_title(window, title)))
    goto error;
  if (!SYS_IS_OK(sys_window_set_geometry(window, x, y, width, height)))
    goto error;

  // The kernel sends many messages at initialization. Most of the window
  // creation is done when receiving these messages, so poll all of them now.
  // In particular, the framebuffer is first allocated when the SYS_MSG_RESIZED
  // is received.
  sys_poll_all_messages(window);
  return window;

error:
  sys_window_destroy(window);
  return NULL;
}

void sys_window_destroy(sys_window_t* window) {
  if (window == NULL)
    return;

  free(window->title);

  // Free the window inside the kernel.
  if (window->kernel_handle == 0)
    __syscall1(SYS_WINDOW_DESTROY, window->kernel_handle);

  free(window);
}

static sys_error_t handle_message(sys_window_t* window, sys_message_t* msg) {
  (void)window;

  switch (msg->id) {
    default:
      break;
  }

  return SYS_ERR_OK;
}

sys_bool_t sys_poll_message(sys_window_t* window, sys_message_t* msg) {
  assert(window != NULL);

  sys_word_t result = __syscall2(SYS_POLL_MESSAGE, window->kernel_handle, (sys_word_t)msg);
  if (!SYS_IS_OK(result))
    return sys_false;

  handle_message(window, msg);
  return sys_true;
}

void sys_poll_all_messages(sys_window_t* window) {
  sys_message_t message;
  while (sys_poll_message(window, &message))
    continue;
}

void sys_wait_message(sys_window_t* window, sys_message_t* msg) {
  assert(window != NULL);
  __syscall2(SYS_WAIT_MESSAGE, window->kernel_handle, (sys_word_t)msg);
  handle_message(window, msg);
}

sys_error_t sys_window_set_title(sys_window_t* window, const char* title) {
  assert(window != NULL && title != NULL);

  // Update the local (userspace) copy of the window title.
  free(window->title);
  const size_t title_length = strlen(title) + 1 /* include NUL terminator */;
  window->title = (char*)malloc(sizeof(char) * title_length);
  if (window->title == NULL)
    return SYS_ERR_OUT_OF_MEM;
  memcpy(window->title, title, sizeof(char) * title_length);

  // Inform the kernel of the change.
  return __syscall2(SYS_WINDOW_SET_TITLE, window->kernel_handle, (sys_word_t)window->title);
}

sys_error_t sys_window_set_visibility(sys_window_t* window, sys_bool_t visible) {
  assert(window != NULL);
  return __syscall2(SYS_WINDOW_SET_VISIBILITY, window->kernel_handle, visible);
}

sys_error_t sys_window_get_visibility(sys_window_t* window, sys_bool_t* visible) {
  assert(window != NULL);
  return __syscall2(SYS_WINDOW_GET_VISIBILITY, window->kernel_handle, (sys_word_t)visible);
}

sys_error_t sys_window_set_geometry(sys_window_t* window, int32_t x, int32_t y, uint32_t width, uint32_t height) {
  assert(window != NULL);

  return __syscall5(SYS_WINDOW_SET_GEOMETRY, window->kernel_handle, x, y, width, height);
}

sys_error_t sys_window_get_geometry(sys_window_t* window, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height) {
  assert(window != NULL);

  return __syscall5(SYS_WINDOW_GET_GEOMETRY, window->kernel_handle, (sys_word_t)x, (sys_word_t)y, (sys_word_t)width,
                    (sys_word_t)height);
}

sys_error_t sys_window_present(sys_window_t* window) {
  assert(window != NULL);

  return __syscall1(SYS_WINDOW_PRESENT, window->kernel_handle);
}

sys_error_t sys_gfx_clear(sys_window_t* window, uint32_t argb) {
  assert(window != NULL);
  return __syscall2(SYS_GFX_CLEAR, window->kernel_handle, argb);
}

sys_error_t sys_gfx_draw_line(sys_window_t* window, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t argb) {
  assert(window != NULL);

  const uint64_t param1 = (uint64_t)x0 | ((uint64_t)y0 << 32);
  const uint64_t param2 = (uint64_t)x1 | ((uint64_t)y1 << 32);
  return __syscall4(SYS_GFX_DRAW_LINE, window->kernel_handle, param1, param2, argb);
}

sys_error_t sys_gfx_draw_rect(sys_window_t* window,
                              uint32_t x,
                              uint32_t y,
                              uint32_t width,
                              uint32_t height,
                              uint32_t argb) {
  assert(window != NULL);

  const uint64_t param1 = (uint64_t)x | ((uint64_t)y << 32);
  const uint64_t param2 = (uint64_t)width | ((uint64_t)height << 32);
  return __syscall4(SYS_GFX_DRAW_RECT, window->kernel_handle, param1, param2, argb);
}

sys_error_t sys_gfx_fill_rect(sys_window_t* window,
                              uint32_t x,
                              uint32_t y,
                              uint32_t width,
                              uint32_t height,
                              uint32_t argb) {
  assert(window != NULL);

  const uint64_t param1 = (uint64_t)x | ((uint64_t)y << 32);
  const uint64_t param2 = (uint64_t)width | ((uint64_t)height << 32);
  return __syscall4(SYS_GFX_FILL_RECT, window->kernel_handle, param1, param2, argb);
}

sys_error_t sys_gfx_draw_text(sys_window_t* window, uint32_t x, uint32_t y, const char* text, uint32_t argb) {
  assert(window != NULL);

  const uint64_t param1 = (uint64_t)x | ((uint64_t)y << 32);
  return __syscall4(SYS_GFX_DRAW_TEXT, window->kernel_handle, param1, (sys_word_t)text, argb);
}
