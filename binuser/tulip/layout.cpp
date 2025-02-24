#include "layout.hpp"
#include <cstddef>
#include <cstdint>

TuLayout::TuLayout(TuWidget* parent) {
  if (parent != nullptr) {
    parent->set_layout(this);
  } else {
    m_geometry = {{0, 0}, {0, 0}};
    m_parent = nullptr;
  }
}

TuBoxLayout::TuBoxLayout(TuWidget* parent, bool is_horizontal) : TuLayout(parent), m_is_horizontal(is_horizontal) {}

void TuBoxLayout::add_widget(TuWidget* widget, int stretch) {
  m_items.push_back({widget, stretch});
}

void TuBoxLayout::update() {
  // Calculate the minimum size of the layout.
  uint32_t min_size = 0;
  if (m_is_horizontal) {  // Horizontal layout
    for (const auto& item : m_items) {
      min_size += item.widget->get_min_size().width;
    }
  } else {  // Vertical layout
    for (const auto& item : m_items) {
      min_size += item.widget->get_min_size().height;
    }
  }

  // Add spacing between items.
  min_size += m_spacing * (m_items.get_size() - 1);

  // Calculate the remaining space.
  int32_t remaining_space;
  if (m_is_horizontal) {
    remaining_space = (int32_t)(get_geometry().size.width - min_size);
  } else {
    remaining_space = (int32_t)(get_geometry().size.height - min_size);
  }

  // Distribute the remaining space among the stretchable items.
  remaining_space = TuMax(remaining_space, 0);
  if (m_is_horizontal) {  // Horizontal layout
    int32_t x = (int32_t)get_geometry().origin.x;
    const int32_t y = (int32_t)get_geometry().origin.y;
    for (auto& item : m_items) {
      const int32_t extra_space = (int32_t)(((size_t)remaining_space * item.stretch) / m_items.get_size());
      item.widget->resize(item.widget->get_min_size().width + extra_space, get_geometry().size.height);
      item.widget->move(x, y);
      x += (int32_t)(item.widget->get_size().width + m_spacing);
    }
  } else {  // Vertical layout
    const int32_t x = (int32_t)get_geometry().origin.x;
    int32_t y = (int32_t)get_geometry().origin.y;
    for (auto& item : m_items) {
      const int32_t extra_space = (int32_t)(((size_t)remaining_space * item.stretch) / m_items.get_size());
      item.widget->resize(get_geometry().size.width, item.widget->get_min_size().height + extra_space);
      item.widget->move(x, y);
      y += (int32_t)(item.widget->get_size().height + m_spacing);
    }
  }
}
