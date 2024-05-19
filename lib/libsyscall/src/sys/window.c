#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/window.h>

struct __sys_window_t {
  // Internal handle to identify the window inside the kernel.
  sys_word_t kernel_handle;
  uint32_t flags;

  // The saved window title.
  char* title;

  // The userspace framebuffer.
  uint32_t* framebuffer;
  uint32_t width, height, pitch;
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

  window->kernel_handle = __syscall0(SYS_WINDOW_CREATE);
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
  free(window->framebuffer);

  // Free the window inside the kernel.
  if (window->kernel_handle == 0)
    __syscall1(SYS_WINDOW_DESTROY, window->kernel_handle);

  free(window);
}

static sys_error_t reallocate_framebuffer(sys_window_t* window, uint32_t width, uint32_t height) {
  assert(window != NULL);

  uint32_t* new_framebuffer = (uint32_t*)malloc(sizeof(uint32_t) * width * height);
  if (new_framebuffer == NULL)
    return SYS_ERR_OUT_OF_MEM;

  free(window->framebuffer);
  window->framebuffer = new_framebuffer;
  window->width = width;
  window->height = height;
  window->pitch = width;

  __syscall2(SYS_WINDOW_SET_FRAMEBUFFER, window->kernel_handle, (sys_word_t)window->framebuffer);
  return SYS_ERR_OK;
}

static sys_error_t handle_message(sys_window_t* window, sys_message_t* msg) {
  switch (msg->id) {
    case SYS_MSG_RESIZE:
      return reallocate_framebuffer(window, msg->param1, msg->param2);
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

  free(window->title);
  size_t title_length = strlen(title) + 1 /* include NUL terminator */;
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

sys_error_t sys_window_set_geometry(sys_window_t* window, int32_t x, int32_t y, uint32_t width, uint32_t height) {
  assert(window != NULL);

  return __syscall5(SYS_WINDOW_SET_GEOMETRY, window->kernel_handle, x, y, width, height);
}

sys_error_t sys_window_get_geometry(sys_window_t* window, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height) {
  assert(window != NULL);

  return __syscall5(SYS_WINDOW_GET_GEOMETRY, window->kernel_handle, (sys_word_t)x, (sys_word_t)y, (sys_word_t)width,
                    (sys_word_t)height);
}

uint32_t* sys_window_get_framebuffer(sys_window_t* window, uint32_t* pitch) {
  assert(window != NULL);

  if (pitch != NULL)
    *pitch = window->pitch;

  return window->framebuffer;
}

enum { FRAME_THICKNESS = 1, TITLE_BAR_HEIGHT = 20 };

sys_error_t sys_window_get_client_area(sys_window_t* window,
                                       uint32_t* x,
                                       uint32_t* y,
                                       uint32_t* width,
                                       uint32_t* height) {
  assert(window != NULL);

  // First retrieve the window geometry.
  sys_error_t error = sys_window_get_geometry(window, x, y, width, height);
  if (!SYS_IS_OK(error))
    return error;

  if ((window->flags & SYS_WF_NO_FRAME) != 0) {
    // If we do not need to draw a window frame, then the client area is the same
    // as the window geometry. So no need to update the values here.
    return SYS_ERR_OK;
  }

  // Otherwise, the client area is the window geometry minus the area needed to
  // draw the window frame (borders + title bar).
  if (x != NULL)
    *x += FRAME_THICKNESS;
  if (y != NULL)
    *y += TITLE_BAR_HEIGHT + FRAME_THICKNESS;
  if (width != NULL)
    *width -= FRAME_THICKNESS * 2;
  if (height != NULL)
    *height -= TITLE_BAR_HEIGHT + FRAME_THICKNESS * 2;

  return SYS_ERR_OK;
}

void sys_window_clear(sys_window_t* window) {
  assert(window != NULL && window->framebuffer != NULL);

  size_t framebuffer_size = sizeof(uint32_t) * window->height * window->pitch;
  memset(window->framebuffer, 0, framebuffer_size);
}

static void draw_window_frame(sys_window_t* window) {}

#if 0
sys_error_t sys_window_present(sys_window_t* window) {
  assert(window != NULL && window->framebuffer != NULL);

  // Draw the window frame.
  if ((window->flags & SYS_WF_NO_FRAME) == 0) {
    draw_window_frame(window);
  }

  // Send the updated framebuffer to the kernel.
  return __syscall5(SYS_WINDOW_PRESENT, window->kernel_handle, (sys_word_t)window->framebuffer, window->width,
                    window->height, window->pitch);
}
#endif
