#pragma once

#include <libk/memory.hpp>
#include <libk/string_view.hpp>
#include "graphics/graphics.hpp"
#include "memory/buffer.hpp"
#include "task/task.hpp"
#include "wm/geometry.hpp"
#include "wm/message_queue.hpp"

class Window {
 public:
  static constexpr int32_t MIN_WIDTH = 25;
  static constexpr int32_t MIN_HEIGHT = 25;
#ifdef CONFIG_WINDOW_LARGE_FRAMEBUFFER
  // Lower limits when all windows use a full framebuffer (to limit excessive memory usage).
  static constexpr int32_t MAX_WIDTH = 2000;
  static constexpr int32_t MAX_HEIGHT = 2000;
#else
  static constexpr int32_t MAX_WIDTH = UINT16_MAX;
  static constexpr int32_t MAX_HEIGHT = UINT16_MAX;
#endif  // CONFIG_WINDOW_LARGE_FRAMEBUFFER
  static constexpr size_t MAX_TITLE_LENGTH = 255;

  Window(const libk::SharedPointer<Task>& task);

  /** Gets the owner task of this window. All windows have an owner. */
  [[nodiscard]] libk::SharedPointer<Task> get_task() const { return m_task; }

  /** Gets the UTF-8 encoded title of this window (to be displayed). */
  [[nodiscard]] libk::StringView get_title() const { return m_title; }
  /** A copy of @a title is made internally by the window. */
  void set_title(libk::StringView title);

  [[nodiscard]] bool is_visible() const { return m_visible; }
  void show() { set_visibility(true); }
  void hide() { set_visibility(false); }
  void set_visibility(bool visible) { m_visible = visible; }

  [[nodiscard]] bool has_focus() const { return m_focus; }

  [[nodiscard]] Rect get_geometry() const { return m_geometry; }
  void set_geometry(const Rect& rect);

  [[nodiscard]] MessageQueue& get_message_queue() { return m_message_queue; }
  [[nodiscard]] const MessageQueue& get_message_queue() const { return m_message_queue; }

  // Graphics functions:
#ifdef CONFIG_USE_DMA
  [[nodiscard]] uint32_t* get_framebuffer() { return (uint32_t*)m_framebuffer->get(); }
  [[nodiscard]] const uint32_t* get_framebuffer() const { return (const uint32_t*)m_framebuffer->get(); }
  [[nodiscard]] DMA::Address get_framebuffer_dma_addr() const { return m_framebuffer->get_dma_address(); }
#else
  [[nodiscard]] uint32_t* get_framebuffer() { return m_framebuffer; }
  [[nodiscard]] const uint32_t* get_framebuffer() const { return m_framebuffer; }
#endif  // CONFIG_USE_DMA
  [[nodiscard]] uint32_t get_framebuffer_pitch() const { return m_framebuffer_pitch; }
  void clear(uint32_t argb = 0x000000);
  void draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t argb);
  void draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t argb);
  void fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t argb);
  void draw_text(uint32_t x, uint32_t y, const char* text, uint32_t argb);
  // Draw the window frame decoration (title bar + borders).
  void draw_frame();

 private:
  void reallocate_framebuffer();

 private:
  friend class WindowManager;

  // The task that owns this window. A window is always owned by a unique task.
  // When the task is killed, all child windows are destroyed.
  libk::SharedPointer<Task> m_task;

  // The window-specific message queue (messages from the keyboard driver,
  // the window manager, users, etc.).
  MessageQueue m_message_queue;

  // The window allocates the title pointer on the kernel side.
  // It is free when the window is destroyed or when the title is modified.
  libk::StringView m_title;

  // The window size and position (relative to the screen).
  Rect m_geometry;

  // The framebuffer is allocated on the kernel side. It is updated each time
  // the window geometry changes. The size of the framebuffer is the same
  // as the window size (see m_geometry variable).
#ifdef CONFIG_USE_DMA
  libk::ScopedPointer<Buffer> m_framebuffer;
#else
  uint32_t* m_framebuffer;
#endif
  uint32_t m_framebuffer_pitch = 0;

  graphics::Painter m_painter;

  // Some flags about the window:
  bool m_has_frame : 1 = true;  // should we draw the window frame (title bar + borders)?
  bool m_visible : 1 = false;   // the window is currently visible?
  bool m_focus : 1 = false;     // the window currently has the focus (receives keyboard inputs)?
};  // class Window
