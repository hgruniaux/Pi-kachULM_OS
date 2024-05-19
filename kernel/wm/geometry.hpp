#pragma once

#include <cstdint>

struct Rect {
  int32_t x1, y1, x2, y2;

  Rect() = default;
  Rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2) : x1(x1), y1(y1), x2(x2), y2(y2) { normalize(); }

  void normalize() {
    if (x2 < x1)
      std::swap(x1, x2);
    if (y2 < y1)
      std::swap(y1, y2);
  }

  [[nodiscard]] bool is_null() const { return width() == 0 || height() == 0; }
  [[nodiscard]] bool has_surface() const { return width() > 0 && height() > 0; }

  [[nodiscard]] int32_t left() const { return x1; }
  [[nodiscard]] int32_t right() const { return x2; }
  [[nodiscard]] int32_t top() const { return y1; }
  [[nodiscard]] int32_t bottom() const { return y2; }

  [[nodiscard]] int32_t x() const { return x1; }
  [[nodiscard]] int32_t y() const { return y1; }
  [[nodiscard]] int32_t width() const { return x2 - x1; }
  [[nodiscard]] int32_t height() const { return y2 - y1; }

  void set_left(int32_t left) {
    x1 = left;
    if (x2 < x1)
      std::swap(x1, x2);
  }

  void set_right(int32_t right) {
    x2 = right;
    if (x2 < x1)
      std::swap(x1, x2);
  }

  void set_top(int32_t top) {
    y1 = top;
    if (y2 < y1)
      std::swap(y1, y2);
  }

  void set_bottom(int32_t bottom) {
    y2 = bottom;
    if (y2 < y1)
      std::swap(y1, y2);
  }

  void set_width(uint32_t width) { x2 = x1 + (int32_t)width; }
  void set_height(uint32_t height) { x2 = x1 + (int32_t)height; }

  bool intersects(const Rect& r) const {
    if (is_null() || r.is_null())
      return false;

    int l1 = x1;
    int r1 = x1;
    if (x2 - x1 + 1 < 0)
      l1 = x2;
    else
      r1 = x2;
    int l2 = r.x1;
    int r2 = r.x1;
    if (r.x2 - r.x1 + 1 < 0)
      l2 = r.x2;
    else
      r2 = r.x2;
    if (l1 > r2 || l2 > r1)
      return false;
    int t1 = y1;
    int b1 = y1;
    if (y2 - y1 + 1 < 0)
      t1 = y2;
    else
      b1 = y2;
    int t2 = r.y1;
    int b2 = r.y1;
    if (r.y2 - r.y1 + 1 < 0)
      t2 = r.y2;
    else
      b2 = r.y2;
    if (t1 > b2 || t2 > b1)
      return false;
    return true;
  }

  static Rect from_edges(int32_t left, int32_t top, int32_t right, int32_t bottom) {
    return {left, top, right, bottom};
  }

  static Rect from_pos_and_size(int32_t x, int32_t y, int32_t width, int32_t height) {
    return {x, y, x + width, y + height};
  }
};  // struct Rect
