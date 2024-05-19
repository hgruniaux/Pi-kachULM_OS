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

  /* Window messages. */
  SYS_MSG_SHOW,
  SYS_MSG_HIDE,
  SYS_MSG_REPAINT,
  SYS_MSG_CLOSE,
  SYS_MSG_MOVE,
  SYS_MSG_RESIZE,
  SYS_MSG_FOCUS_IN,
  SYS_MSG_FOCUS_OUT,
};

enum { SYS_WF_DEFAULT = 0x0, SYS_WF_NO_FRAME = 0x1 };

sys_window_t* sys_window_create(const char* __title,
                                int32_t __x,
                                int32_t __y,
                                uint32_t __width,
                                uint32_t __height,
                                uint32_t __flags);
void sys_window_destroy(sys_window_t* __w);

sys_bool_t sys_poll_message(sys_window_t* __w, sys_message_t* __msg);
void sys_poll_all_messages(sys_window_t* __w);
void sys_wait_message(sys_window_t* __w, sys_message_t* __msg);

sys_error_t sys_window_set_title(sys_window_t* __w, const char* __title);
sys_error_t sys_window_set_visibility(sys_window_t* __w, sys_bool_t __v);
sys_error_t sys_window_set_geometry(sys_window_t* __w, int32_t __x, int32_t __y, uint32_t __width, uint32_t __height);
sys_error_t sys_window_get_geometry(sys_window_t* __w,
                                    uint32_t* __x,
                                    uint32_t* __y,
                                    uint32_t* __width,
                                    uint32_t* __height);

uint32_t* sys_window_get_framebuffer(sys_window_t* __w, uint32_t* __pitch);
sys_error_t sys_window_get_client_area(sys_window_t* __w,
                                       uint32_t* __x,
                                       uint32_t* __y,
                                       uint32_t* __width,
                                       uint32_t* __height);
void sys_window_clear(sys_window_t* __w);

__SYS_EXTERN_C_END

#endif  // !__PIKAOS_LIBC_SYS_WINDOW_H__
