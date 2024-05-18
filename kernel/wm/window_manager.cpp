#include "wm/window_manager.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/timer.hpp"
#include "task/task.hpp"
#include "wm/window.hpp"

#include <algorithm>

WindowManager* WindowManager::g_instance = nullptr;

WindowManager::WindowManager() {
  KASSERT(g_instance == nullptr && "multiple window manager created");
  g_instance = this;

  auto& fb = FrameBuffer::get();
  m_screen_width = fb.get_width();
  m_screen_height = fb.get_height();
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
  return window;
}

void WindowManager::destroy_window(Window* window) {
  KASSERT(is_valid(window));

  window->get_task()->unregister_window(window);

  const auto it = std::find(m_windows.begin(), m_windows.end(), window);
  m_windows.erase(it);

  if (m_focus_window == window) {
    // Unfocus the window.
    if (!m_windows.is_empty())
      m_focus_window = *m_windows.begin();
    else
      m_focus_window = nullptr;
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
  rect.w = libk::clamp(rect.w, Window::MIN_WIDTH, Window::MAX_WIDTH);
  rect.h = libk::clamp(rect.h, Window::MIN_HEIGHT, Window::MAX_HEIGHT);

  // Handle special values.
  if (rect.x == SYS_POS_DEFAULT) {
    rect.x = 50;
  } else if (rect.x == SYS_POS_CENTERED) {
    rect.x = m_screen_width / 2 - rect.w / 2;
  }

  if (rect.y == SYS_POS_DEFAULT) {
    rect.y = 50;
  } else if (rect.x == SYS_POS_CENTERED) {
    rect.y = m_screen_height / 2 - rect.h / 2;
  }

  // Enforce constraints on window position.
  rect.x = libk::clamp(rect.x, INT16_MIN, INT16_MAX);
  rect.y = libk::clamp(rect.y, INT16_MIN, INT16_MAX);

  const auto old_rect = window->get_geometry();

  window->set_geometry(rect);

  const bool moved = old_rect.x != rect.x || old_rect.y != rect.y;
  const bool resized = old_rect.w != rect.w || old_rect.h != rect.h;

  // Send move and resize messages.
  if (moved) {
    sys_message_t message;
    libk::bzero(&message, sizeof(sys_message_t));
    message.id = SYS_MSG_MOVE;
    message.param1 = rect.x;
    message.param2 = rect.y;
    post_message(window, message);
  }

  if (resized) {
    sys_message_t message;
    libk::bzero(&message, sizeof(sys_message_t));
    message.id = SYS_MSG_RESIZE;
    message.param1 = rect.w;
    message.param2 = rect.h;
    post_message(window, message);
  }

  draw_background(old_rect);
}

[[gnu::hot]] void WindowManager::blit_image_with_resize(const uint32_t* src,
                                                        uint32_t src_width,
                                                        uint32_t src_height,
                                                        uint32_t src_pitch,
                                                        uint32_t src_depth,
                                                        uint32_t* dst,
                                                        uint32_t dst_width,
                                                        uint32_t dst_height,
                                                        uint32_t dst_pitch) {
  for (uint32_t x = 0; x < dst_width; ++x) {
    for (uint32_t y = 0; y < dst_height; ++y) {
      if (m_depth_buffer[x + m_screen_width * y] > src_depth)
        continue;

      const uint32_t src_x = x * (src_width / dst_width);
      const uint32_t src_y = y * (src_height / dst_height);
      dst[x + dst_pitch * y] = src[src_x + src_pitch * src_y];
      m_depth_buffer[x + m_screen_width * y] = src_depth;
    }
  }
}

[[gnu::hot]] void WindowManager::blit_image(const uint32_t* src,
                                            uint32_t src_width,
                                            uint32_t src_height,
                                            uint32_t src_pitch,
                                            uint32_t src_depth,
                                            uint32_t* dst,
                                            uint32_t dst_pitch) {
  for (uint32_t x = 0; x < src_width; ++x) {
    for (uint32_t y = 0; y < src_height; ++y) {
      if (m_depth_buffer[x + m_screen_width * y] > src_depth)
        continue;

      dst[x + dst_pitch * y] = src[x + src_pitch * y];
      m_depth_buffer[x + m_screen_width * y] = src_depth;
    }
  }
}

void WindowManager::present_window(Window* window,
                                   const uint32_t* framebuffer,
                                   uint32_t w,
                                   uint32_t h,
                                   uint32_t pitch) {
  KASSERT(is_valid(window));
  KASSERT(framebuffer != nullptr);

  // Do not present anything if the window is invisible.
  if (!window->is_visible())
    return;

  // TODO: Handle window clipping
  const uint32_t window_x = libk::max(0, window->m_geometry.x);
  const uint32_t window_y = libk::max(0, window->m_geometry.y);

  auto& screen_framebuffer = FrameBuffer::get();
  const auto screen_pitch = screen_framebuffer.get_pitch();
  auto* target_screen_buffer = &screen_framebuffer.get_buffer()[window_x + screen_pitch * window_y];

  const bool has_framebuffer_correct_size = (window->m_geometry.w == w && window->m_geometry.h == h);
  if (!has_framebuffer_correct_size) {
    // The framebuffer has different size as the window. It needs to be resized
    // before blitting it to the screen.
    blit_image_with_resize(framebuffer, w, h, pitch, window->m_depth, target_screen_buffer, window->m_geometry.w,
                           window->m_geometry.h, screen_pitch);
  } else {
    blit_image(framebuffer, w, h, pitch, window->m_depth, target_screen_buffer, screen_pitch);
  }
}

void WindowManager::draw_background(const Rect& rect) {
  auto& screen_framebuffer = FrameBuffer::get();
  const auto screen_pitch = screen_framebuffer.get_pitch();
  auto* target_screen_buffer = screen_framebuffer.get_buffer();

  for (uint32_t x = rect.left(); x < rect.right(); ++x) {
    for (uint32_t y = rect.top(); y < rect.bottom(); ++y) {
      if (m_depth_buffer[x + m_screen_width * y] > 0)
        continue;

      target_screen_buffer[x + screen_pitch * y] = 0xffffff;
    }
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
  size_t nb_columns = libk::isqrt(m_window_count);  // isqrt returns the floor value
  if (nb_columns * nb_columns != m_window_count)
    nb_columns += 1;  // convert it into ceil(sqrt(...))

  const size_t nb_rows = libk::div_round_up(m_window_count, nb_columns);

  const uint32_t window_width = m_screen_width / nb_columns;
  const uint32_t window_height = m_screen_height / nb_rows;

  uint32_t i = 0, j = 0;
  for (auto* window : m_windows) {
    const auto x = (int32_t)(i * window_width);
    const auto y = (int32_t)(j * window_height);
    set_window_geometry(window, {x, y, window_width, window_height});

    ++i;
    if (i >= nb_columns) {
      i = 0;
      ++j;
    }
  }
}
