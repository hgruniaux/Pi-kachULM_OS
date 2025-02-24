#ifndef __PIKAOS_LIBC_SYS_WINDOW_H__
#define __PIKAOS_LIBC_SYS_WINDOW_H__

#include "__types.h"
#include "__utils.h"

__SYS_EXTERN_C_BEGIN

typedef struct __sys_window_t sys_window_t;

typedef struct __sys_message_t {
  uint32_t id;
  uint32_t timestamp;
  uint64_t param1;
  uint64_t param2;
} sys_message_t;

#define SYS_POS_DEFAULT INT32_MIN
#define SYS_POS_CENTERED (INT32_MIN + 1)

enum {
  SYS_MSG_NULL,

  /* Keyboard messages. */
  SYS_MSG_KEYDOWN,
  SYS_MSG_KEYUP,

  /* Mouse messages. */
  SYS_MSG_MOUSEMOVE,
  SYS_MSG_MOUSECLICK,
  SYS_MSG_MOUSESCROLL,

  /* Window messages. */
  SYS_MSG_SHOW,
  SYS_MSG_HIDE,
  SYS_MSG_CLOSE,
  SYS_MSG_MOVE,
  SYS_MSG_RESIZE,
  SYS_MSG_FOCUS_IN,
  SYS_MSG_FOCUS_OUT,
};

typedef enum sys_mouse_button_t {
  SYS_MOUSE_BUTTON_LEFT = 0,
  SYS_MOUSE_BUTTON_MIDDLE = 1,
  SYS_MOUSE_BUTTON_RIGHT = 2,
} sys_mouse_button_t;

enum { SYS_WF_DEFAULT = 0x0, SYS_WF_NO_FRAME = 0x1 };

/* Window creation and destruction API. */
sys_window_t* sys_window_create(const char* title,
                                int32_t x,
                                int32_t y,
                                uint32_t width,
                                uint32_t height,
                                uint32_t flags);
void sys_window_destroy(sys_window_t* window);

/* Window message queue API. */
sys_bool_t sys_poll_message(sys_window_t* window, sys_message_t* msg);
void sys_poll_all_messages(sys_window_t* window);
void sys_wait_message(sys_window_t* window, sys_message_t* msg);

/* Window title (UTF-8 encoded) API. */
sys_error_t sys_window_set_title(sys_window_t* window, const char* title);

/* Window visibility (hidden or shown) API. */
sys_error_t sys_window_set_visibility(sys_window_t* window, sys_bool_t visible);
sys_error_t sys_window_get_visibility(sys_window_t* window, sys_bool_t* visible);

/* Window geometry (position and size) API. */
sys_error_t sys_window_set_geometry(sys_window_t* window, int32_t x, int32_t y, uint32_t width, uint32_t height);
sys_error_t sys_window_get_geometry(sys_window_t* window, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height);

/* Window graphics API. */
sys_error_t sys_window_present(sys_window_t* window);
sys_error_t sys_window_present2(sys_window_t* window, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
sys_error_t sys_gfx_clear(sys_window_t* window, uint32_t argb);
sys_error_t sys_gfx_draw_line(sys_window_t* window, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y2, uint32_t argb);
sys_error_t sys_gfx_draw_rect(sys_window_t* window,
                              uint32_t x,
                              uint32_t y,
                              uint32_t width,
                              uint32_t height,
                              uint32_t argb);
sys_error_t sys_gfx_fill_rect(sys_window_t* window,
                              uint32_t x,
                              uint32_t y,
                              uint32_t width,
                              uint32_t height,
                              uint32_t argb);
sys_error_t sys_gfx_draw_text(sys_window_t* window, uint32_t x, uint32_t y, const char* text, uint32_t argb);

sys_error_t sys_gfx_blit(sys_window_t* window,
                         uint32_t x,
                         uint32_t y,
                         uint32_t width,
                         uint32_t height,
                         const uint32_t* argb_buffer);

__SYS_EXTERN_C_END

#endif  // !__PIKAOS_LIBC_SYS_WINDOW_H__
