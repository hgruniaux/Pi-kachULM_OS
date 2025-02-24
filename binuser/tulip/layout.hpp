#ifndef TULIP_LAYOUT_HPP
#define TULIP_LAYOUT_HPP

#include "vector.hpp"
#include "widget.hpp"

/**
 * A layout is a way to arrange children widgets in a parent widget.
 */
class TuLayout {
 public:
  TuLayout(TuWidget* parent);
  virtual ~TuLayout() = default;

  /** Gets the parent widget of this layout. */
  [[nodiscard]] TuWidget* get_parent() const { return m_parent; }

  /** Gets the geometry of this layout. */
  [[nodiscard]] const TuRect& get_geometry() const { return m_geometry; }
  /** Sets the geometry of this layout. */
  void set_geometry(const TuRect& geometry) { m_geometry = geometry; }

  virtual void update() = 0;

 private:
  friend class TuWidget;
  TuWidget* m_parent;
  TuRect m_geometry;
};  // class TuLayout

/**
 * A layout that lines up widgets in a row or column.
 *
 * @see TuHBoxLayout and TuVBoxLayout
 */
class TuBoxLayout : public TuLayout {
 public:
  TuBoxLayout(TuWidget* parent, bool is_horizontal);

  void add_widget(TuWidget* widget, int stretch = 1);

  void update() override;

 private:
  struct BoxItem {
    TuWidget* widget;
    int stretch;
  };  // struct BoxItem

  TuVector<BoxItem> m_items;
  uint32_t m_spacing = 0;
  bool m_is_horizontal;
};  // class TuBoxLayout

/**
 * A layout that lines up widgets horizontally.
 */
class TuHBoxLayout : public TuBoxLayout {
 public:
  TuHBoxLayout(TuWidget* parent) : TuBoxLayout(parent, true) {}
};  // class TuHBoxLayout

/**
 * A layout that lines up widgets vertically.
 */
class TuVBoxLayout : public TuBoxLayout {
 public:
  TuVBoxLayout(TuWidget* parent) : TuBoxLayout(parent, false) {}
};  // class TuVBoxLayout

#endif  // TULIP_LAYOUT_HPP
