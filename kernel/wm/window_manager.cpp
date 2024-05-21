#include "wm/window_manager.hpp"
#include "data/wallpaper.hpp"
#include "graphics/graphics.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/timer.hpp"
#include "libk/log.hpp"
#include "wm/window.hpp"

#include <algorithm>

#ifdef CONFIG_USE_DMA
#include "hardware/dma/request.hpp"
#endif  // CONFIG_USE_DMA

#include "data/wallpaper.hpp"
#include "graphics/stb_image.h"

WindowManager* WindowManager::g_instance = nullptr;

WindowManager::WindowManager() {
  KASSERT(g_instance == nullptr && "multiple window manager created");
  g_instance = this;

  auto& fb = FrameBuffer::get();
  m_is_supported = fb.is_initialized();
  if (m_is_supported) {
    m_screen_buffer = fb.get_buffer();
    m_screen_width = fb.get_width();
    m_screen_height = fb.get_height();
    m_screen_pitch = fb.get_pitch();
    m_screen_buffer = fb.get_buffer();
  }

#ifdef CONFIG_INCLUDE_WALLPAPER
  int wallpaper_width, wallpaper_height;
  m_wallpaper = (const uint32_t*)stbi_load_from_memory(wallpaper_jpg, wallpaper_jpg_len, &wallpaper_width,
                                                       &wallpaper_height, nullptr, 4);
  m_wallpaper_width = wallpaper_width;
  m_wallpaper_height = wallpaper_height;
#endif  // CONFIG_INCLUDE_WALLPAPER
}

[[nodiscard]] bool WindowManager::is_valid(Window* window) const {
  if (window == nullptr)
    return false;

  const auto it = std::find(m_windows.begin(), m_windows.end(), window);
  return it != m_windows.end();
}

Window* WindowManager::create_window(const libk::SharedPointer<Task>& task, uint32_t flags) {
  if (!m_is_supported)
    return nullptr;

  Window* window = new Window(task);
  if (window == nullptr)
    return nullptr;

  if ((flags & SYS_WF_NO_FRAME) != 0)
    window->m_has_frame = false;

  task->register_window(window);
  ++m_window_count;
  m_windows.push_back(window);

  if (m_focus_window == nullptr)
    focus_window(window);

  m_dirty = true;
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

  m_dirty = true;
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

  m_dirty = true;
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

  m_dirty = true;
}

void WindowManager::focus_window(Window* window) {
  KASSERT(is_valid(window));

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
  m_focus_window->m_focus = true;

  // TODO: Move focus window to front
  auto it = std::find(m_windows.begin(), m_windows.end(), window);
  KASSERT(it != m_windows.end());
  m_windows.erase(it);
  m_windows.push_front(window);
  m_dirty = true;
}

void WindowManager::unfocus_window(Window* window) {
  KASSERT(is_valid(window));

  if (m_focus_window != window)
    return;

  if (m_focus_window != nullptr) {
    m_focus_window->m_focus = false;

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
  if (!m_is_supported)
    return;

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

void WindowManager::present_window(Window* window) {
  KASSERT(is_valid(window));

  (void)window;

  // Let do a full update the next time.
  // A partial update is more challenging as we should only draw the pixels that are visible on the screen.
  // This is straightforward if the window is a top level window (e.g. has focus),
  // but it becomes tricky if the window is behind other windows.
  m_dirty = true;  // mark the screen as dirty and needs an update
}

void WindowManager::mosaic_layout() {
  if (m_window_count == 0)
    return;

  // Let nb_columns be ceil(sqrt(m_window_count)).
  int nb_columns = libk::isqrt(m_window_count);  // isqrt returns the floor value
  if ((size_t)(nb_columns) * (size_t)(nb_columns) != m_window_count)
    nb_columns += 1;  // convert it into ceil(sqrt(...))

  const int nb_rows = (int)libk::div_round_up(m_window_count, nb_columns);

  const int window_width = m_screen_width / nb_columns;
  const int window_height = m_screen_height / nb_rows;

  int current_idx = 0;
  int i = 0, j = 0;
  for (auto* window : m_windows) {
    const auto x = i * window_width;
    const auto y = j * window_height;

    int32_t width = window_width;
    // If we are the last window, take all the remaining place in the last row.
    const bool is_last = (size_t)(current_idx + 1) == m_window_count;
    if (is_last) {
      width = (nb_columns - i) * window_width;
    }

    set_window_geometry(window, Rect::from_pos_and_size(x, y, width, window_height));

    ++i;
    if (i >= nb_columns) {
      i = 0;
      ++j;
    }

    ++current_idx;
  }
}

void WindowManager::update() {
  if (GenericTimer::get_elapsed_time_in_ms() > 5000 && GenericTimer::get_elapsed_time_in_ms() < 7000)
    mosaic_layout();

  if (!m_dirty || !m_is_supported)
    return;

  const auto start = GenericTimer::get_elapsed_time_in_micros();
  draw_windows();
  const auto end = GenericTimer::get_elapsed_time_in_micros();

  m_dirty = false;
  LOG_DEBUG("Window manager update done in {} ms for {} window(s)", (end - start) / 1000, m_window_count);
}

void WindowManager::draw_background(const Rect& rect, DMARequestQueue& request_queue) {
#ifdef CONFIG_INCLUDE_WALLPAPER
#if defined(CONFIG_USE_DMA) && 0 /* disable DMA for the background as it does not work yet */
  const auto framebuffer_dma_addr = DMA::get_dma_bus_address((VirtualAddress)m_wallpaper, true);
  const auto screen_dma_addr = DMA::get_dma_bus_address((VirtualAddress)&m_screen_buffer, false);
  const auto src_stride = 0;
  const auto dst_stride = sizeof(uint32_t) * (m_screen_pitch - m_screen_width);
  auto* request = DMA::Request::memcpy_2d(framebuffer_dma_addr, screen_dma_addr, sizeof(uint32_t) * (m_wallpaper_width),
                                          m_wallpaper_height, src_stride, dst_stride);
  request_queue.add(request);
#else
  (void)request_queue;

  for (int32_t x = rect.left(); x < rect.right(); ++x) {
    for (int32_t y = rect.top(); y < rect.bottom(); ++y) {
      // The wallpaper is in RGBA, we expect ABGR.
      const auto color = libk::bswap(m_wallpaper[x + m_wallpaper_width * y]);
      m_screen_buffer[x + m_screen_pitch * y] = color >> 8;
    }
  }
#endif  // CONFIG_USE_DMA
#else
  (void)request_queue;
  fill_rect(rect, 0xffffff);
#endif  // CONFIG_INCLUDE_WALLPAPER
}

void WindowManager::fill_rect(const Rect& rect, uint32_t color) {
  // TODO: replace this by an optimized function
  for (int32_t x = rect.left(); x < rect.right(); ++x) {
    for (int32_t y = rect.top(); y < rect.bottom(); ++y) {
      m_screen_buffer[x + m_screen_pitch * y] = color;
    }
  }
}

void WindowManager::draw_window(Window* window, const Rect& dst_rect, DMARequestQueue& request_queue) {
#ifndef CONFIG_USE_DMA
  (void)request_queue;
#endif  // !CONFIG_USE_DMA

  const Rect& src_rect = window->m_geometry;
  if (!src_rect.intersects(dst_rect))
    return;

  window->draw_frame();

  const uint32_t* framebuffer = window->get_framebuffer();
  const uint32_t framebuffer_pitch = window->get_geometry().width();
  KASSERT(framebuffer != nullptr);

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
#if CONFIG_USE_DMA
  const auto framebuffer_dma_addr =
      window->get_framebuffer_dma_addr() + sizeof(uint32_t) * (x1 + framebuffer_pitch * y1);
  const auto screen_dma_addr = DMA::get_dma_bus_address(
      (VirtualAddress)&m_screen_buffer[src_rect.x() + x1 + m_screen_pitch * (src_rect.y() + y1)], false);
  const auto src_stride = sizeof(uint32_t) * (framebuffer_pitch - (x2 - x1));
  const auto dst_stride = sizeof(uint32_t) * (m_screen_pitch - (x2 - x1));
  auto* request = DMA::Request::memcpy_2d(framebuffer_dma_addr, screen_dma_addr, sizeof(uint32_t) * (x2 - x1), y2 - y1,
                                          src_stride, dst_stride);
  request_queue.add(request);
#else
  for (uint32_t src_x = x1, dst_x = src_rect.x() + x1; src_x < x2; ++src_x, dst_x++) {
    for (uint32_t src_y = y1, dst_y = src_rect.y() + y1; src_y < y2; ++src_y, dst_y++) {
      const auto color = framebuffer[src_x + framebuffer_pitch * src_y];
      m_screen_buffer[dst_x + m_screen_pitch * dst_y] = color;
    }
  }
#endif  // CONFIG_USE_DMA

  // Draw the focus border to inform the user what window has the focus.
  if (window->has_focus()) {
    graphics::Painter painter(m_screen_buffer, m_screen_width, m_screen_height, m_screen_pitch);
    painter.set_clipping(dst_rect.x1, dst_rect.y1, dst_rect.x2, dst_rect.y2);
    const auto window_rect = window->get_geometry();
    painter.draw_rect(window_rect.x() - 1, window_rect.y() - 1, window_rect.width() + 2, window_rect.height() + 2,
                      0xAA6BA4B8);
  }
}

void WindowManager::draw_windows(libk::LinkedList<Window*>::Iterator it,
                                 const Rect& dst_rect,
                                 DMARequestQueue& request_queue) {
  KASSERT(dst_rect.left() >= 0 && dst_rect.right() <= m_screen_width && dst_rect.top() >= 0 &&
          dst_rect.bottom() <= m_screen_height);

#ifndef CONFIG_USE_DMA
  (void)request_queue;
#endif  // !CONFIG_USE_DMA

  if (!dst_rect.has_surface())
    return;

  if (it == m_windows.end()) {
    // No more windows, we hit the background image!
    // draw_background(dst_rect);
    return;  // end of recursion
  }

  Rect src_rect = (*it)->m_geometry;

  // Draw the current window (into the destination rectangle):
  draw_window(*it, dst_rect, request_queue);

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

  draw_windows(it, left, request_queue);
  draw_windows(it, top, request_queue);
  draw_windows(it, right, request_queue);
  draw_windows(it, bottom, request_queue);
}

void WindowManager::draw_windows() {
#ifdef CONFIG_USE_DMA
  m_dma_channel.abort_previous();
#endif  // CONFIG_USE_DMA

  DMARequestQueue dma_request_queue;

#if CONFIG_USE_NAIVE_WM_UPDATE
  draw_background({0, 0, m_screen_width, m_screen_height}, dma_request_queue);

  if (m_windows.is_empty())
    return;

  auto it = m_windows.begin();
  while (it.has_next())
    ++it;

  while (it != m_windows.end()) {
    draw_window(*it, {0, 0, m_screen_width, m_screen_height}, dma_request_queue);
    --it;
  }
#else
  draw_background({0, 0, m_screen_width, m_screen_height}, dma_request_queue);
  draw_windows(m_windows.begin(), {0, 0, m_screen_width, m_screen_height}, dma_request_queue);
#endif  // CONFIG_USE_NAIVE_WM_UPDATE

#ifdef CONFIG_USE_DMA
  dma_request_queue.execute_and_wait(m_dma_channel);
#endif  // CONFIG_USE_DMA

  FrameBuffer::get().present();
}
