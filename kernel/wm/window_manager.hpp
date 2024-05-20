#pragma once

#include <sys/window.h>
#include <libk/linked_list.hpp>
#include <libk/memory.hpp>
#include "geometry.hpp"
#include "task/task.hpp"

class Window;
class Task;

class WindowManager {
 public:
  WindowManager();

  [[nodiscard]] static WindowManager& get() { return *g_instance; }

  [[nodiscard]] bool is_valid(Window* window) const;

  Window* create_window(const libk::SharedPointer<Task>& task, uint32_t flags);
  void destroy_window(Window* window);

  void set_window_visibility(Window* window, bool visible);
  void set_window_geometry(Window* window, Rect rect);

  void update();

  void focus_window(Window* window);
  void unfocus_window(Window* window);

  void post_message(sys_message_t message);
  bool post_message(Window* window, sys_message_t message);

  void mosaic_layout();

 private:
  void fill_rect(const Rect& rect, uint32_t color);
  void draw_background(const Rect& rect);
  void draw_window(Window* window, const Rect& dst_rect);
  void draw_windows(libk::LinkedList<Window*>::Iterator it, const Rect& dst_rect);
  void draw_windows();

 private:
  static WindowManager* g_instance;
  Window* m_focus_window = nullptr;  // the window that actually has focus
  libk::LinkedList<Window*> m_windows;
  size_t m_window_count = 0;

  uint64_t m_last_update = 0;
  bool m_dirty = true;  // true when the windows need to be redrawn/updated.

  uint32_t* m_screen_buffer;
  size_t m_screen_pitch;
  int32_t m_screen_width, m_screen_height;
};  // class WindowManager
