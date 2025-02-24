#pragma once

#include <sys/window.h>
#include <libk/linked_list.hpp>
#include <libk/memory.hpp>
#include "geometry.hpp"
#include "sys/keyboard.h"
#include "task/task.hpp"

#ifdef CONFIG_USE_DMA
#include "hardware/dma/channel.hpp"
#endif  // CONFIG_USE_DMA

class Window;
class Task;

class WindowManager {
 public:
  WindowManager();

  [[nodiscard]] static WindowManager& get() { return *g_instance; }

  /** Checks if the window manager is supported (screen connected). */
  [[nodiscard]] bool is_supported() const { return m_is_supported; }

  /** Checks if the given window is a valid window, registered in this window manager. */
  [[nodiscard]] bool is_valid(Window* window) const;

  Window* create_window(const libk::SharedPointer<Task>& task, uint32_t flags);
  void destroy_window(Window* window);

  void set_window_visibility(Window* window, bool visible);
  void set_window_geometry(Window* window, Rect rect);
  void set_window_geometry(Window* window, int32_t x, int32_t y, int32_t w, int32_t h);

  void update();
  void update(const Rect& rect);

  void clip_rect_to_screen(Rect& rect);

  void focus_window(Window* window);
  void unfocus_window(Window* window);

  void post_message(sys_message_t message);
  bool post_message(Window* window, sys_message_t message);

  void present_window(Window* window);
  void present_window(Window* window, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

  void mosaic_layout();

 private:
  // Input events handler.
#ifdef CONFIG_HAS_CURSOR
  bool handle_mouse_move_event(int32_t dx, int32_t dy);
  bool handle_mouse_click_event(int button_type, bool is_pressed);
#endif // CONFIG_HAS_CURSOR
  bool handle_key_event(sys_key_event_t event);

  void switch_focus();
  void move_focus_window(int32_t dx, int32_t dy);
  void resize_focus_window(int32_t dx, int32_t dy);

  void read_wallpaper();

#ifdef CONFIG_USE_DMA
  struct DMARequestQueue {
    DMA::Request* first_request = nullptr;
    DMA::Request* last_request = nullptr;

    ~DMARequestQueue() {
      // Free the DMA request queue.
      auto* request = first_request;
      while (request != nullptr) {
        auto* next = request->next();
        delete request;
        request = next;
      }
    }

    void add(DMA::Request* request) {
      KASSERT(request != nullptr);

      if (first_request == nullptr) {
        first_request = request;
        last_request = request;
      } else {
        last_request->link_to(request);
        last_request = request;
      }
    }

    void execute_and_wait(DMA::Channel& channel) {
      if (first_request == nullptr)
        return;

      channel.execute_requests(first_request);
      channel.wait();
    }
  };
#else
  // Stub class when DMA usage is disabled.
  struct DMARequestQueue {};  // struct DMARequestQueue
#endif  // CONFIG_USE_DMA

  // Drawing functions.
  void draw_background(const Rect& rect, DMARequestQueue& request_queue);
  void draw_window(Window* window, const Rect& rect, DMARequestQueue& request_queue);
  void draw_windows_helper(libk::LinkedList<Window*>::Iterator it, const Rect& rect, DMARequestQueue& request_queue);
  void draw_windows(const Rect& rect, DMARequestQueue& request_queue);

#ifdef CONFIG_HAS_CURSOR
  void draw_cursor();
#endif // CONFIG_HAS_CURSOR

 private:
  static WindowManager* g_instance;

  Window* m_focus_window = nullptr;  // the window that actually has focus
  libk::LinkedList<Window*> m_windows;
  size_t m_window_count = 0;

  uint32_t m_last_window_x = 50, m_last_window_y = 50;

#if defined(CONFIG_USE_DMA) && defined(CONFIG_USE_DMA_FOR_WALLPAPER)
  libk::ScopedPointer<Buffer> m_wallpaper;
#else
  const uint32_t* m_wallpaper = nullptr;
#endif  // CONFIG_USE_DMA && CONFIG_USE_DMA_FOR_WALLPAPER
  uint32_t m_wallpaper_width, m_wallpaper_height;

#ifdef CONFIG_USE_DMA
  DMA::Channel m_dma_channel;
#endif  // CONFIG_USE_DMA

  uint32_t* m_screen_buffer = nullptr;
#ifdef CONFIG_USE_DMA
  VirtualAddress m_screen_buffer_dma_addr;
#endif  // CONFIG_USE_DMA
  size_t m_screen_pitch;
  int32_t m_screen_width, m_screen_height;

#ifdef CONFIG_HAS_CURSOR
  int32_t m_cursor_x = 0, m_cursor_y = 0;
#endif // CONFIG_HAS_CURSOR

  bool m_is_supported = true;  // is the window manager supported (screen connected)?
};  // class WindowManager
