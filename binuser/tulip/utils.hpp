#ifndef TULIP_UTILS_HPP
#define TULIP_UTILS_HPP

#include <cstdint>

template <class T>
T TuMin(T a, T b) {
  return a < b ? a : b;
}

template <class T>
T TuMax(T a, T b) {
  return a > b ? a : b;
}

template <class T>
T TuClamp(T value, T min, T max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

struct TuPoint {
  int32_t x;
  int32_t y;

  [[nodiscard]] bool operator==(const TuPoint& other) const { return x == other.x && y == other.y; }
  [[nodiscard]] bool operator!=(const TuPoint& other) const { return !(*this == other); }
};  // struct TuPoint

struct TuSize {
  uint32_t width;
  uint32_t height;

  [[nodiscard]] bool operator==(const TuSize& other) const { return width == other.width && height == other.height; }
  [[nodiscard]] bool operator!=(const TuSize& other) const { return !(*this == other); }
};  // struct TuSize

struct TuRect {
  TuPoint origin;
  TuSize size;

  /** Checks if the given point is inside this rectangle. */
  [[nodiscard]] bool contains(TuPoint point) const { return contains(point.x, point.y); }
  /** Checks if the given point is inside this rectangle. */
  [[nodiscard]] bool contains(int32_t x, int32_t y) const {
    return x >= origin.x && x < (int32_t)(origin.x + size.width) && y >= origin.y && y < (int32_t)(origin.y + size.height);
  }

  void union_with(const TuRect& other) {
    int32_t x1 = TuMin(origin.x, other.origin.x);
    int32_t y1 = TuMin(origin.y, other.origin.y);
    int32_t x2 = TuMax(origin.x + (int32_t)size.width, other.origin.x + (int32_t)other.size.width);
    int32_t y2 = TuMax(origin.y + (int32_t)size.height, other.origin.y + (int32_t)other.size.height);
    origin = {x1, y1};
    size = {static_cast<uint32_t>(x2 - x1), static_cast<uint32_t>(y2 - y1)};
  }
};  // struct TuRect

struct TuMargins {
  uint32_t left;
  uint32_t top;
  uint32_t right;
  uint32_t bottom;
};  // struct TuMargins

struct TuColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;

  [[nodiscard]] uint32_t to_uint32() const { return (a << 24) | (r << 16) | (g << 8) | b; }
};  // struct TuColor

#endif  // TULIP_UTILS_HPP
