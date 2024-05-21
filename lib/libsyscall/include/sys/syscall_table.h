#ifndef __PIKAOS_LIBC_SYS_SYSCALL_TABLE_H__
#define __PIKAOS_LIBC_SYS_SYSCALL_TABLE_H__

enum {
  /* Process system calls. */
  SYS_EXIT,
  SYS_PRINT,
  SYS_GETPID,
  SYS_DEBUG,
  SYS_SPAWN,

  /* Scheduler system calls. */
  SYS_SLEEP,
  SYS_YIELD,
  SYS_SCHED_SET_PRIORITY,
  SYS_SCHED_GET_PRIORITY,

  /* Memory and heap segment system calls. */
  SYS_SBRK,

  /* Filesystem system calls. */

  /* Window manager system calls. */
  SYS_POLL_MESSAGE,
  SYS_WAIT_MESSAGE,
  SYS_WINDOW_CREATE,
  SYS_WINDOW_DESTROY,
  SYS_WINDOW_SET_TITLE,
  SYS_WINDOW_GET_VISIBILITY,
  SYS_WINDOW_SET_VISIBILITY,
  SYS_WINDOW_GET_GEOMETRY,
  SYS_WINDOW_SET_GEOMETRY,
  SYS_WINDOW_PRESENT,

  /* Window graphics system calls. */
  SYS_GFX_CLEAR,
  SYS_GFX_DRAW_LINE,
  SYS_GFX_DRAW_RECT,
  SYS_GFX_FILL_RECT,
  SYS_GFX_DRAW_TEXT,
};

#endif  // !__PIKAOS_LIBC_SYS_SYSCALL_TABLE_H__
