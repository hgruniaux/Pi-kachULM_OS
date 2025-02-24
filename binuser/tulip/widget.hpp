#ifndef TULIP_WIDGET_HPP
#define TULIP_WIDGET_HPP

#include "event.hpp"
#include "utils.hpp"

class TuWindow;
class TuEvent;
class TuLayout;
struct TuStyle;

class TuWidget {
 public:
  TuWidget(TuWindow* window);
  TuWidget(TuWidget* parent);
  virtual ~TuWidget();

  /** Gets the parent of this widget. May be null, if it has no parents. */
  [[nodiscard]] TuWidget* get_parent() const { return m_parent; }
  /** Sets the parent of this widget. */
  void set_parent(TuWidget* parent);

  /** Gets the window that contains this widget. */
  [[nodiscard]] TuWindow* get_window() const { return m_window; }

  /** Gets the current size of the widget. */
  [[nodiscard]] TuSize get_size() const { return m_size; }
  /** Shorthand for `get_size().width` */
  [[nodiscard]] uint32_t get_width() const { return m_size.width; }
  /** Shorthand for `get_size().height` */
  [[nodiscard]] uint32_t get_height() const { return m_size.height; }
  /** Sets the size of the widget. The exact size of the widget after the call may not be @a size. */
  void resize(TuSize size);
  void resize(uint32_t width, uint32_t height) { resize({width, height}); }

  /** Gets the minimum size of the widget. */
  [[nodiscard]] TuSize get_min_size() const { return m_min_size; }
  /** Sets the minimum size of the widget. */
  void set_min_size(TuSize size);

  /** Gets the maximum size of the widget. */
  [[nodiscard]] TuSize get_max_size() const { return m_max_size; }
  /** Sets the maximum size of the widget. */
  void set_max_size(TuSize size);

  /** Gets the position of the widget (relative to its parent). */
  [[nodiscard]] TuPoint get_position() const { return m_position; }
  /** Shorthand for `get_position().x` */
  [[nodiscard]] int32_t get_x() const { return m_position.x; }
  /** Shorthand for `get_position().y` */
  [[nodiscard]] int32_t get_y() const { return m_position.y; }
  /** Sets the position of the widget (relative to its parent). */
  void move(TuPoint position);
  void move(int32_t x, int32_t y) { move({x, y}); }

  /** Gets the padding of this widget. */
  [[nodiscard]] const TuMargins& get_padding() const { return m_padding; }
  /** Sets the margins of this widget. */
  void set_padding(TuMargins padding) { m_padding = padding; }
  /** Sets the margins of this widget. */
  void set_padding(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom) {
    set_padding({left, top, right, bottom});
  }

  /** Gets the internal geometry of this widget (that is Rect(0, 0, width, height)). */
  [[nodiscard]] TuRect get_rect() const { return {{0, 0}, m_size}; }
  /** Gets the geometry of this widget relative to its parent (that is Rect(x, y, width, height)). */
  [[nodiscard]] TuRect get_geometry() const { return {m_position, m_size}; }

  void repaint() { repaint(0, 0, m_size.width, m_size.height); }
  void repaint(const TuRect& rect) { repaint(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height); }
  void repaint(int32_t x, int32_t y, uint32_t width, uint32_t height);

  /** Gets the style used by this widget. */
  [[nodiscard]] TuStyle* get_style() const { return m_style; }
  void set_style(TuStyle* style) { m_style = style; }

  /** Gets the layout used by this widget. This may be null. */
  [[nodiscard]] TuLayout* get_layout() const { return m_layout; }
  /** Sets the layout to be used by this widget.  */
  void set_layout(TuLayout* layout);

  /** Checks if this widget has the keyboard focus. */
  [[nodiscard]] bool has_focus() const { return m_has_keyboard_focus; }

 protected:
  virtual void on_event(TuEvent* event);
  virtual void on_paint_event(TuPaintEvent* event) { (void)event; }
  virtual void on_move_event(TuMoveEvent* event) { (void)event; }
  virtual void on_resize_event(TuResizeEvent* event) { (void)event; }
  virtual void on_focus_in_event(TuEvent* event) { (void)event; }
  virtual void on_focus_out_event(TuEvent* event) { (void)event; }

 private:
  void add_child(TuWidget* child);
  void remove_child(TuWidget* child);
  void invalidate_parent_layout();

 private:
  TuWindow* m_window = nullptr;
  TuWidget* m_parent = nullptr;

  TuWidget* m_children = nullptr;
  TuWidget* m_next_sibling = nullptr;

  TuStyle* m_style = nullptr;
  TuLayout* m_layout = nullptr;

  TuPoint m_position = {0, 0};
  TuSize m_size = {0, 0};
  TuSize m_min_size = {0, 0};
  TuSize m_max_size = {UINT32_MAX, UINT32_MAX};

  TuMargins m_padding = {0, 0, 0, 0};

  // Does this widget have the keyboard focus?
  bool m_has_keyboard_focus = false;
  // Can this widget have the keyboard focus? For example, a label
  // cannot have the keyboard focus but a button can.
  bool m_can_have_keyboard_focus = false;
};  // class TuWidget

#endif  // TULIP_WIDGET_HPP
