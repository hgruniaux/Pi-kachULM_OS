#include "widget.hpp"
#include "application.hpp"
#include "layout.hpp"
#include "window.hpp"

#include <sys/syscall.h>

TuWidget::TuWidget(TuWindow* window) {
  m_window = window;
  m_style = TuApplication::get()->get_style();
  m_padding = {m_style->window_padding.width, m_style->window_padding.height, m_style->window_padding.width,
               m_style->window_padding.height};
  m_window->set_content(this);
}

TuWidget::TuWidget(TuWidget* parent) {
  set_parent(parent);

  if (parent == nullptr) {
    m_style = TuApplication::get()->get_style();
  }

  m_padding = {m_style->frame_padding.width, m_style->frame_padding.height, m_style->frame_padding.width,
               m_style->frame_padding.height};
}

TuWidget::~TuWidget() {
  // Remove this widget from the parent's list of children.
  if (m_parent != nullptr) {
    m_parent->remove_child(this);
  }

  // Delete all children.
  TuWidget* child = m_children;
  while (child != nullptr) {
    TuWidget* next = child->m_next_sibling;
    delete child;
    child = next;
  }
}

void TuWidget::set_parent(TuWidget* parent) {
  if (m_parent == parent)
    return;

  // Remove this widget from the current parent.
  if (m_parent != nullptr) {
    m_parent->remove_child(this);
  }

  m_parent = parent;

  // Add this widget to the new parent.
  if (m_parent != nullptr) {
    m_parent->add_child(this);
  }

  // Inherits some properties from parent.
  if (m_parent != nullptr) {
    m_window = m_parent->m_window;
    m_style = m_parent->m_style;
  } else {
    m_window = nullptr;
  }
}

void TuWidget::add_child(TuWidget* child) {
  if (m_children == nullptr) {
    m_children = child;
    return;
  }

  TuWidget* last_child = m_children;
  while (last_child->m_next_sibling != nullptr) {
    last_child = last_child->m_next_sibling;
  }

  last_child->m_next_sibling = child;
}

void TuWidget::remove_child(TuWidget* child) {
  if (m_children == child) {
    m_children = child->m_next_sibling;
    return;
  }

  TuWidget* prev = m_children;
  while (prev->m_next_sibling != child) {
    prev = prev->m_next_sibling;
  }

  prev->m_next_sibling = child->m_next_sibling;
}

void TuWidget::set_min_size(TuSize size) {
  m_min_size = size;

  TuSize new_size = m_size;
  new_size.width = TuClamp(new_size.width, m_min_size.width, m_max_size.width);
  new_size.height = TuClamp(new_size.height, m_min_size.height, m_max_size.height);
  resize(new_size);
  invalidate_parent_layout();
}

void TuWidget::set_max_size(TuSize size) {
  m_max_size = size;

  TuSize new_size = m_size;
  new_size.width = TuClamp(new_size.width, m_min_size.width, m_max_size.width);
  new_size.height = TuClamp(new_size.height, m_min_size.height, m_max_size.height);
  resize(new_size);
  invalidate_parent_layout();
}

void TuWidget::resize(TuSize size) {
  size.width = TuClamp(size.width, m_min_size.width, m_max_size.width);
  size.height = TuClamp(size.height, m_min_size.height, m_max_size.height);

  if (size == m_size)
    return;  // No change

  TuResizeEvent event(m_size, size);
  m_size = size;
  on_event(&event);
}

void TuWidget::move(TuPoint position) {
  if (position == m_position)
    return;  // No change

  TuMoveEvent event(m_position, position);
  m_position = position;
  on_event(&event);
}

void TuWidget::repaint(int32_t x, int32_t y, uint32_t width, uint32_t height) {
  TuRect paint_rect;
  paint_rect.origin.x = TuClamp(x, 0, (int32_t)m_size.width);
  paint_rect.origin.y = TuClamp(y, 0, (int32_t)m_size.height);
  paint_rect.size.width = TuClamp<uint32_t>(width, 0, m_size.width - paint_rect.origin.x);
  paint_rect.size.height = TuClamp<uint32_t>(height, 0, m_size.height - paint_rect.origin.y);

  TuPaintEvent event(paint_rect);
  on_event(&event);
}

void TuWidget::set_layout(TuLayout* layout) {
  if (m_layout == layout)
    return;

  if (m_layout != nullptr) {
    delete m_layout;
  }

  sys_print("TuWidget::set_layout: setting layout\n");
  m_layout = layout;
  if (m_layout != nullptr) {
    m_layout->m_parent = this;
    m_layout->set_geometry(get_rect());
    m_layout->update();
  }
}

void TuWidget::on_event(TuEvent* event) {
  switch (event->get_type()) {
    case TuEvent::Type::PAINT: {
      // Firt we paint the widget itself.
      on_paint_event(static_cast<TuPaintEvent*>(event));

      // Then, we need to find if there are any children that need to be painted.
      TuWidget* child = m_children;
      while (child != nullptr) {
        auto paint_rect = child->get_rect();
        TuPaintEvent child_paint_event(paint_rect);
        child->on_event(&child_paint_event);
        child = child->m_next_sibling;
      }
    } break;

    case TuEvent::Type::MOVE: {
      // When a widget is moved, its children are not moved relative to their parent.
      // Therefore, we don't need to propagate the event to the children.
      on_move_event(static_cast<TuMoveEvent*>(event));
    } break;

    case TuEvent::Type::RESIZE: {
      // When a widget is resized, its children are not automatically resized.
      // Therefore, we don't need to propagate the event to the children.
      // However, the widget's implementation may choose to resize its children.
      on_resize_event(static_cast<TuResizeEvent*>(event));

      if (m_layout != nullptr) {
        m_layout->set_geometry(get_rect());
        m_layout->update();
      }

    } break;
    default:
      break;
  }
}

void TuWidget::invalidate_parent_layout() {
  if (m_parent == nullptr || m_parent->m_layout == nullptr)
    return;

  m_parent->m_layout->update();
}
