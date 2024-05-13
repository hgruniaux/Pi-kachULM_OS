#include <syscall/syscall.h>

// sys_error_t __syscall0(uint32_t);
.global __syscall0
__syscall0:
    mov w8, w0
    svc #0
    ret

// sys_error_t __syscall1(uint32_t, uint64_t);
.global __syscall1
__syscall1:
  mov w8, w0
  mov x0, x1
  svc #0
  ret

// sys_error_t sys_debug(uint64_t);
.global sys_debug
sys_debug:
  mov w8, #SYS_DEBUG
  svc #0
  ret

// sys_error_t sys_gfx_draw_line(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
.global sys_gfx_draw_line
sys_gfx_draw_line:
  mov w8, #SYS_GFX_DRAW_LINE
  svc #0
  ret

// sys_error_t sys_gfx_draw_rect(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
.global sys_gfx_draw_rect
sys_gfx_draw_rect:
  mov w8, #SYS_GFX_DRAW_RECT
  svc #0
  ret

// sys_error_t sys_gfx_fill_line(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
.global sys_gfx_fill_line
sys_gfx_fill_line:
  mov w8, #SYS_GFX_FILL_RECT
  svc #0
  ret

// sys_error_t sys_gfx_draw_text(uint32_t, uint32_t, const char*, uint32_t);
.global sys_gfx_draw_text
sys_gfx_draw_text:
  mov w8, #SYS_GFX_DRAW_TEXT
  svc #0
  ret
