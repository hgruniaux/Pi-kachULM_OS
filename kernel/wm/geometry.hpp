#pragma once

#include <cstdint>

struct Rect {
  int32_t x, y;
  uint32_t w, h;

  [[nodiscard]] int32_t left() const { return x; }
  [[nodiscard]] int32_t right() const { return x + (int32_t)w; }
  [[nodiscard]] int32_t top() const { return y; }
  [[nodiscard]] int32_t bottom() const { return y + (int32_t)h; }

  /** Checks if this rectangle overlaps with @a other. */
  [[nodiscard]] bool overlap(const Rect& other) const {
    return left() < other.right() && right() > other.left() && top() > other.bottom() && bottom() < other.top();
  }
};  // struct Rect
