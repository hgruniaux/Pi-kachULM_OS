#ifndef TULIP_WINDOW_HPP
#define TULIP_WINDOW_HPP

#include <sys/window.h>
#include "utils.hpp"

class TuWidget;
class TuTitleBar;
class TuRootWidget;

class TuWindow {
public:
  TuWindow(const char* title, uint32_t width, uint32_t height);
  ~TuWindow();

  /** Returns true if the window is valid. */
  [[nodiscard]] bool is_valid() const { return m_window != nullptr; }

  /** Returns the internal system window handle. */
  [[nodiscard]] sys_window_t* get_handle() const { return m_window; }

  /** Returns the title of the window. */
  [[nodiscard]] const char* get_title() const { return m_title; }
  void set_title(const char* title);

  /** Returns true if the window should close. */
  [[nodiscard]] bool should_close() const { return m_should_close; }
  void set_should_close(bool should_close) { m_should_close = should_close; }

  /** Returns true if the window has the focus. */
  [[nodiscard]] bool has_focus() const { return m_has_focus; }

  /** Returns the position of the window. */
  [[nodiscard]] TuPoint get_position() const { return m_position; }
  /** Shorthand for `get_position().x` */
  [[nodiscard]] int32_t get_x() const { return m_position.x; }
  /** Shorthand for `get_position().y` */
  [[nodiscard]] int32_t get_y() const { return m_position.y; }
  /** Sets the position of the window. */
  void move(TuPoint position);
  void move(int32_t x, int32_t y) { move({x, y}); }

  /** Returns the size of the window. */
  [[nodiscard]] TuSize get_size() const { return m_size; }
  /** Shorthand for `get_size().width` */
  [[nodiscard]] uint32_t get_width() const { return m_size.width; }
  /** Shorthand for `get_size().height` */
  [[nodiscard]] uint32_t get_height() const { return m_size.height; }
  /** Sets the size of the window. */
  void resize(TuSize size);
  void resize(uint32_t width, uint32_t height) { resize({width, height}); }

  void set_content(TuWidget* widget);

  void present() { present(0, 0, m_size.width, m_size.height); }
  void present(const TuRect& rect) {  present(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height); }
  void present(int32_t x, int32_t y, uint32_t width, uint32_t height);

private:
  void poll_system_messages();

private:
  friend class TuApplication;
  sys_window_t* m_window = nullptr;
  TuWindow* m_next = nullptr;

  const char* m_title = nullptr;

  // Internal widget that defines all the window's area.
  // It contains the content widget and the title bar.
  TuRootWidget* m_root_widget = nullptr;

  TuPoint m_position;
  TuSize m_size;

  bool m_should_close = false;
  bool m_has_focus = false;
}; // class TuWindow

#endif // TULIP_WINDOW_HPP
