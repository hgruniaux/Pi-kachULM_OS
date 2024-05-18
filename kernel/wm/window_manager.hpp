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

  Window* create_window(const libk::SharedPointer<Task>& task);
  void destroy_window(Window* window);

  void set_window_visibility(Window* window, bool visible);
  void set_window_geometry(Window* window, Rect rect);

  void present_window(Window* window, const uint32_t* framebuffer, uint32_t w, uint32_t h, uint32_t pitch);

  void focus_window(Window* window);
  void unfocus_window(Window* window);

  void post_message(sys_message_t message);
  bool post_message(Window* window, sys_message_t message);

  void mosaic_layout();

 private:
  void blit_image_with_resize(const uint32_t* src,
                              uint32_t src_width,
                              uint32_t src_height,
                              uint32_t src_pitch,
                              uint32_t src_depth,
                              uint32_t* dst,
                              uint32_t dst_width,
                              uint32_t dst_height,
                              uint32_t dst_pitch);
  void blit_image(const uint32_t* src,
                  uint32_t src_width,
                  uint32_t src_height,
                  uint32_t src_pitch,
                  uint32_t src_depth,
                  uint32_t* dst,
                  uint32_t dst_pitch);

  void repaint_windows(const Rect& rect);
  void draw_background(const Rect& rect);

 private:
  static WindowManager* g_instance;
  Window* m_focus_window = nullptr;  // the window that actually has focus
  libk::LinkedList<Window*> m_windows;
  size_t m_window_count = 0;
  uint32_t m_screen_width, m_screen_height;

  // The depth buffer, same size as the screen.
  uint8_t* m_depth_buffer;
};  // class WindowManager
