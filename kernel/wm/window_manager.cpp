#include "wm/window_manager.hpp"
#include "graphics/graphics.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/timer.hpp"
#include "libk/log.hpp"
#include "task/task.hpp"
#include "wm/window.hpp"

#include <algorithm>
#include <ranges>

WindowManager* WindowManager::g_instance = nullptr;

WindowManager::WindowManager() {
  KASSERT(g_instance == nullptr && "multiple window manager created");
  g_instance = this;

  auto& fb = FrameBuffer::get();
  m_screen_width = fb.get_width();
  m_screen_height = fb.get_height();
  m_screen_pitch = fb.get_pitch();
  m_screen_buffer = fb.get_buffer();
  m_depth_buffer = new uint8_t[m_screen_width * m_screen_height];
  libk::bzero(m_depth_buffer, sizeof(uint8_t) * m_screen_width * m_screen_height);
}

[[nodiscard]] bool WindowManager::is_valid(Window* window) const {
  if (window == nullptr)
    return false;

  const auto it = std::find(m_windows.begin(), m_windows.end(), window);
  return it != m_windows.end();
}

Window* WindowManager::create_window(const libk::SharedPointer<Task>& task) {
  Window* window = new Window(task);
  if (window == nullptr)
    return nullptr;

  task->register_window(window);
  ++m_window_count;
  m_windows.push_back(window);

  if (m_focus_window == nullptr)
    focus_window(window);

  return window;
}

void WindowManager::destroy_window(Window* window) {
  KASSERT(is_valid(window));

  window->get_task()->unregister_window(window);

  const auto it = std::find(m_windows.begin(), m_windows.end(), window);
  m_windows.erase(it);

  if (m_focus_window == window) {
    // Unfocus the window.
    unfocus_window(window);
  }

  --m_window_count;
  delete window;
}

void WindowManager::set_window_visibility(Window* window, bool visible) {
  KASSERT(is_valid(window));

  if (window->is_visible() == visible)
    return;  // already the correct visibility

  window->m_visible = visible;

  // Update the focus window if needed.
  if (visible) {  // show the window
    if (m_focus_window == nullptr)
      focus_window(window);
  } else {  // hide the window
    if (m_focus_window == window)
      unfocus_window(window);
  }

  // Send message.
  sys_message_t message;
  libk::bzero(&message, sizeof(sys_message_t));
  message.id = visible ? SYS_MSG_SHOW : SYS_MSG_HIDE;
  post_message(window, message);
}

void WindowManager::set_window_geometry(Window* window, Rect rect) {
  KASSERT(is_valid(window));

  // Enforce constraints on window size.
  rect.set_width(libk::clamp(rect.width(), Window::MIN_WIDTH, Window::MAX_WIDTH));
  rect.set_height(libk::clamp(rect.height(), Window::MIN_HEIGHT, Window::MAX_HEIGHT));

  // Handle special values.
  if (rect.x1 == SYS_POS_DEFAULT) {
    rect.x1 = 50;
  } else if (rect.x1 == SYS_POS_CENTERED) {
    rect.x1 = m_screen_width / 2 - rect.width() / 2;
  }

  if (rect.y1 == SYS_POS_DEFAULT) {
    rect.y1 = 50;
  } else if (rect.x1 == SYS_POS_CENTERED) {
    rect.y1 = m_screen_height / 2 - rect.height() / 2;
  }

  // Enforce constraints on window position.
  rect.x1 = (libk::clamp(rect.x(), INT16_MIN, INT16_MAX));
  rect.y1 = (libk::clamp(rect.y(), INT16_MIN, INT16_MAX));

  const auto old_rect = window->get_geometry();

  window->set_geometry(rect);

  const bool moved = old_rect.x() != rect.x() || old_rect.y() != rect.y();
  const bool resized = old_rect.width() != rect.width() || old_rect.height() != rect.height();

  // Send move and resize messages.
  if (moved) {
    sys_message_t message;
    libk::bzero(&message, sizeof(sys_message_t));
    message.id = SYS_MSG_MOVE;
    message.param1 = rect.x();
    message.param2 = rect.y();
    post_message(window, message);
  }

  if (resized) {
    sys_message_t message;
    libk::bzero(&message, sizeof(sys_message_t));
    message.id = SYS_MSG_RESIZE;
    message.param1 = rect.width();
    message.param2 = rect.height();
    post_message(window, message);
  }
}

void WindowManager::focus_window(Window* window) {
  if (m_focus_window == window)
    return;

  // Unfocus the previous window.
  if (m_focus_window != nullptr) {
    unfocus_window(m_focus_window);
  }

  // Send focus in message.
  sys_message_t message;
  libk::bzero(&message, sizeof(sys_message_t));
  message.id = SYS_MSG_FOCUS_IN;
  post_message(window, message);

  m_focus_window = window;

  // TODO: Move focus window to front
  auto it = std::find(m_windows.begin(), m_windows.end(), window);
  KASSERT(it != m_windows.end());
  m_windows.erase(it);
  m_windows.push_front(window);
}

void WindowManager::unfocus_window(Window* window) {
  if (m_focus_window != window)
    return;

  if (m_focus_window != nullptr) {
    // Send focus out messsage.
    sys_message_t message;
    libk::bzero(&message, sizeof(sys_message_t));
    message.id = SYS_MSG_FOCUS_OUT;
    post_message(m_focus_window, message);
  }

  Window* new_focus_window = nullptr;
  for (auto* w : m_windows) {
    // TODO: give focus to the nearest window (according to depth).
    if (w != window)
      w = new_focus_window;
  }

  m_focus_window = nullptr;
  focus_window(new_focus_window);
}

void WindowManager::post_message(sys_message_t message) {
  message.timestamp = GenericTimer::get_elapsed_time_in_ms();
  for (auto* window : m_windows) {
    window->get_message_queue().enqueue(message);
  }
}

bool WindowManager::post_message(Window* window, sys_message_t message) {
  KASSERT(is_valid(window));

  message.timestamp = GenericTimer::get_elapsed_time_in_ms();
  return window->get_message_queue().enqueue(message);
}

void WindowManager::mosaic_layout() {
  // Let nb_columns be ceil(sqrt(m_window_count)).
  int nb_columns = libk::isqrt(m_window_count);  // isqrt returns the floor value
  if ((size_t)(nb_columns) * (size_t)(nb_columns) != m_window_count)
    nb_columns += 1;  // convert it into ceil(sqrt(...))

  const int nb_rows = (int)libk::div_round_up(m_window_count, nb_columns);

  const int window_width = m_screen_width / nb_columns;
  const int window_height = m_screen_height / nb_rows;

  int i = 0, j = 0;
  for (auto* window : m_windows) {
    const auto x = i * window_width;
    const auto y = j * window_height;
    set_window_geometry(window, {x, y, window_width, window_height});

    ++i;
    if (i >= nb_columns) {
      i = 0;
      ++j;
    }
  }
}

void WindowManager::update() {
  const auto start = GenericTimer::get_elapsed_time_in_micros();

  // Avoid too frequent updates...
  // We aim for 60 Hz, so one update per 16 ms.
  if (start - m_last_update < 16000)
    return;
  m_last_update = start;

  draw_windows();
  const auto end = GenericTimer::get_elapsed_time_in_micros();
  LOG_DEBUG("Window manager update done in {} ms", (end - start) / 1000);
}

void WindowManager::draw_background(const Rect& rect) {
  // TODO: replace this by an optimized function
  for (int32_t x = rect.left(); x < rect.right(); ++x) {
    for (int32_t y = rect.top(); y < rect.bottom(); ++y) {
      m_screen_buffer[x + m_screen_pitch * y] = 0xffffff;
    }
  }
}

void WindowManager::fill_rect(const Rect& rect, uint32_t color) {
  // TODO: replace this by an optimized function
  for (int32_t x = rect.left(); x < rect.right(); ++x) {
    for (int32_t y = rect.top(); y < rect.bottom(); ++y) {
      m_screen_buffer[x + m_screen_pitch * y] = color;
    }
  }
}

void WindowManager::draw_window(Window* window, const Rect& dst_rect) {
  const Rect& src_rect = window->m_geometry;
  if (!src_rect.intersects(dst_rect))
    return;

  const uint32_t* framebuffer = window->get_framebuffer();
  const uint32_t framebuffer_pitch = window->get_geometry().width();

  // If the window has no registered framebuffer, then just fill with black.
  if (framebuffer == nullptr) {
    // TODO: replace this by an optimized function
    for (int32_t x = dst_rect.left(); x < dst_rect.right(); ++x) {
      for (int32_t y = dst_rect.top(); y < dst_rect.bottom(); ++y) {
        m_screen_buffer[x + m_screen_pitch * y] = 0;
      }
    }

    return;
  }

  uint32_t x1 = 0, x2 = src_rect.width();
  uint32_t y1 = 0, y2 = src_rect.height();

  // Do clipping to avoid drawing outside the destination rect.
  if (src_rect.left() < dst_rect.left())
    x1 = dst_rect.left() - src_rect.left();
  if (src_rect.top() < dst_rect.top())
    y1 = dst_rect.top() - src_rect.top();
  if (src_rect.right() > dst_rect.right())
    x2 = x1 + dst_rect.right() - libk::max(src_rect.left(), dst_rect.left());
  if (src_rect.bottom() > dst_rect.bottom())
    y2 = y1 + dst_rect.bottom() - libk::max(src_rect.top(), dst_rect.top());

  if (x1 == x2 || y1 == y2)
    return;

  // Blit the framebuffer into the screen.
  // TODO: Maybe use DMA to do the blit for better performances.
  // FIXME: avoid changing virtual memory view here
  window->get_task()->get_saved_state().memory->activate();
  for (uint32_t src_x = x1, dst_x = src_rect.x() + x1; src_x < x2; ++src_x, dst_x++) {
    for (uint32_t src_y = y1, dst_y = src_rect.y() + y1; src_y < y2; ++src_y, dst_y++) {
      const auto color = framebuffer[src_x + framebuffer_pitch * src_y];
      m_screen_buffer[dst_x + m_screen_pitch * dst_y] = color;
    }
  }
}

void WindowManager::draw_windows(libk::LinkedList<Window*>::Iterator it, const Rect& dst_rect) {
  KASSERT(dst_rect.left() >= 0 && dst_rect.right() <= m_screen_width && dst_rect.top() >= 0 &&
          dst_rect.bottom() <= m_screen_height);

  if (!dst_rect.has_surface())
    return;

  if (it == m_windows.end()) {
    // No more windows, we hit the background image!
    // draw_background(dst_rect);
    return;  // end of recursion
  }

  Rect src_rect = (*it)->m_geometry;

  // Draw the current window (into the destination rectangle):
  draw_window(*it, dst_rect);

  // Recursively draw the windows behind:
  // dst_rect:
  //  |---------------------------------------------------------|
  //  |               |         TOP           |                 |
  //  |               |                       |                 |
  //  |               |-----------------------|                 |
  //  |               |                       |                 |
  //  |     LEFT      |        CENTER         |      RIGHT      |
  //  |               |       src_rect        |                 |
  //  |               |-----------------------|                 |
  //  |               |                       |                 |
  //  |               |        BOTTOM         |                 |
  //  |               |                       |                 |
  //  |---------------------------------------------------------|

  if (src_rect.right() > dst_rect.right())
    src_rect.set_right(dst_rect.right());
  if (src_rect.left() < dst_rect.left())
    src_rect.set_left(dst_rect.left());
  if (src_rect.bottom() > dst_rect.bottom())
    src_rect.set_bottom(dst_rect.bottom());
  if (src_rect.top() < dst_rect.top())
    src_rect.set_top(dst_rect.top());

  const Rect left = Rect::from_edges(dst_rect.left(), dst_rect.top(), src_rect.left(), dst_rect.bottom());
  const Rect top = Rect::from_edges(src_rect.left(), dst_rect.top(), src_rect.right(), src_rect.top());
  const Rect right = Rect::from_edges(src_rect.right(), dst_rect.top(), dst_rect.right(), dst_rect.bottom());
  const Rect bottom = Rect::from_edges(src_rect.left(), src_rect.bottom(), src_rect.right(), dst_rect.bottom());

  // get the next visible window (windows are sorted from front to back).
  do {
    it++;
  } while (it != m_windows.end() && !(*it)->is_visible());

  draw_windows(it, left);
  draw_windows(it, top);
  draw_windows(it, right);
  draw_windows(it, bottom);
}

void WindowManager::draw_windows() {
  m_screen_buffer = FrameBuffer::get().get_buffer();

#if 0
  draw_background({0, 0, m_screen_width, m_screen_height});
  draw_windows(m_windows.begin(), {0, 0, m_screen_width, m_screen_height});
#else
  draw_background({0, 0, m_screen_width, m_screen_height});

  if (m_windows.is_empty())
    return;

  auto it = m_windows.begin();
  while (it.has_next())
    ++it;

  while (it != m_windows.end()) {
    draw_window(*it, {0, 0, m_screen_width, m_screen_height});
    --it;
  }
#endif

  FrameBuffer::get().present();
}
