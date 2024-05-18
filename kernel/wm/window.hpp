#pragma once

#include <libk/memory.hpp>
#include <libk/string_view.hpp>
#include "task/task.hpp"
#include "wm/geometry.hpp"
#include "wm/message_queue.hpp"

class Window {
 public:
  static constexpr uint32_t MIN_WIDTH = 25;
  static constexpr uint32_t MIN_HEIGHT = 25;
  static constexpr uint32_t MAX_WIDTH = UINT16_MAX;
  static constexpr uint32_t MAX_HEIGHT = UINT16_MAX;
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
  void set_geometry(const Rect& rect) { m_geometry = rect; }

  [[nodiscard]] MessageQueue& get_message_queue() { return m_message_queue; }
  [[nodiscard]] const MessageQueue& get_message_queue() const { return m_message_queue; }

 private:
  friend class WindowManager;
  libk::SharedPointer<Task> m_task;  // the task that owns this window
  MessageQueue m_message_queue;
  libk::StringView m_title;  // allocated by the Window, owned pointer
  Rect m_geometry;
  uint8_t m_depth;
  bool m_visible : 1 = false;
  bool m_focus : 1 = false;
};  // class Window
