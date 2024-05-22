#include <sys/syscall.h>
#include <sys/window.h>

static void draw_credits(sys_window_t* window) {
  sys_gfx_clear(window, 0xffffff);
  sys_gfx_draw_text(window, 20, 50, "Proudly presented by:", 0x000000);

  uint32_t x = 20, y = 80;
#define DRAW_ITEM(text)                                      \
  sys_gfx_fill_rect(window, x + 5, y + 5, 5, 5, 0xff000000); \
  sys_gfx_draw_text(window, x + 20, y, (text), 0x000000);    \
  y += 20

  DRAW_ITEM("Gabriel Desfrene");
  DRAW_ITEM("Hubert Gruniaux");
  DRAW_ITEM("Emile Sauvat");
}

int main() {
  sys_print("CREDITS");

  sys_window_t* window = sys_window_create("Credits", SYS_POS_DEFAULT, SYS_POS_DEFAULT, 500, 400, SYS_WF_DEFAULT);
  if (window == NULL) {
    sys_print("Failed to create window for credits");
    return 1;
  }

  draw_credits(window);

  bool should_close = false;
  while (!should_close) {
    sys_message_t message;
    sys_wait_message(window, &message);
    switch (message.id) {
      case SYS_MSG_CLOSE:
        should_close = true;
        break;
      case SYS_MSG_RESIZE:
        draw_credits(window);
        break;
      default:
        break;
    }
  }

  sys_window_destroy(window);
  return 0;
}
