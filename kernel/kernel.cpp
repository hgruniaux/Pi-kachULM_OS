#include <libk/log.hpp>
#include "hardware/device.hpp"

#include "graphics/graphics.hpp"
#include "hardware/framebuffer.hpp"
#include "hardware/interrupts.hpp"
#include "hardware/kernel_dt.hpp"
#include "hardware/system_timer.hpp"
#include "hardware/timer.hpp"
#include "hardware/uart.hpp"

#include "hardware/dma/channel.hpp"
#include "hardware/dma/dma_controller.hpp"
#include "hardware/dma/request.hpp"
#include "memory/buffer.hpp"
#include "memory/mem_alloc.hpp"
#include "task/task_manager.hpp"

#if defined(__GNUC__)
#define COMPILER_NAME "GCC " __VERSION__
#elif defined(__clang__)
#define COMPILER_NAME __VERSION__
#else
#define COMPILER_NAME "Unknown Compiler"
#endif

extern "C" const char init[];

[[noreturn]] void kmain() {
  LOG_INFO("Kernel built at " __TIME__ " on " __DATE__ " with " COMPILER_NAME " !");

  LOG_INFO("Board model: {}", KernelDT::get_board_model());
  LOG_INFO("Board revision: {:#x}", KernelDT::get_board_revision());
  LOG_INFO("Board serial: {:#x}", KernelDT::get_board_serial());
  LOG_INFO("Temp: {} °C / {} °C", Device::get_current_temp() / 1000, Device::get_max_temp() / 1000);

  static constexpr size_t width = 1280;
  static constexpr size_t height = 720;

  static constexpr size_t red_width = 2 * width / 3;
  static constexpr size_t red_height = 2 * height / 3;

  static constexpr size_t red_x = 30;
  static constexpr size_t red_y = 30;

  static constexpr size_t green_width = 400;
  static constexpr size_t green_height = 400;

  static constexpr size_t green_x = (width - green_width) / 2;
  static constexpr size_t green_y = (height - green_height) / 2;

  FrameBuffer& framebuffer = FrameBuffer::get();
  if (!framebuffer.init(width, height)) {
    LOG_WARNING("failed to initialize framebuffer");
  } else {
    //    const uint32_t fb_width = framebuffer.get_width();
    //    const uint32_t fb_height = framebuffer.get_height();
    //
    //    graphics::Painter painter;
    //    const char* text = "Hello kernel World from Graphics!";
    //    const PKFont font = painter.get_font();
    //    const uint32_t text_width = font.get_horizontal_advance(text);
    //    const uint32_t text_height = font.get_char_height();
    //
    //    // Draw the text at the middle of screen
    //    painter.clear(graphics::Color::WHITE);
    //    painter.set_pen(graphics::Color::BLACK);
    //    painter.draw_text((fb_width - text_width) / 2, (fb_height - text_height) / 2, text);
    Buffer red_window(red_width * red_height * sizeof(uint32_t));
    uint32_t* red_ptr = (uint32_t*)red_window.get();

    for (size_t i = 0; i < red_width; ++i) {
      for (size_t j = 0; j < red_height; ++j) {
        red_ptr[i + red_width * j] = 0xffff0000;
      }
    }

    Buffer green_window(green_width * green_height * sizeof(uint32_t));
    uint32_t* green_ptr = (uint32_t*)green_window.get();

    for (size_t i = 0; i < green_width; ++i) {
      for (size_t j = 0; j < green_height; ++j) {
        green_ptr[i + green_width * j] = 0xff00ff0f;
      }
    }
    LOG_INFO("We have: {:#x}", green_ptr[green_width - 1 + green_width * 10]);

    const auto red_dma = red_window.get_dma_address();
    const auto green_dma = green_window.get_dma_address();
    const auto fb_dma = DMA::get_dma_bus_address((uintptr_t)framebuffer.get_buffer(), true);

    const auto red_fb_dma = fb_dma + (red_x + width * red_y) * sizeof(uint32_t);
    const auto green_fb_dma = fb_dma + (green_x + width * green_y) * sizeof(uint32_t);

    //    DMA::Request req1 = DMA::Request::memcpy(src_dma, dst_dma, width * height * sizeof(uint32_t));

    DMA::Request* red_req = DMA::Request::memcpy_2d(red_dma, red_fb_dma, red_width * sizeof(uint32_t), red_height - 1,
                                                    0, (width - red_width) * sizeof(uint32_t));

    DMA::Request* green_req = DMA::Request::memcpy_2d(green_dma, green_fb_dma, green_width * sizeof(uint32_t),
                                                      green_height - 1, 0, (width - green_width) * sizeof(uint32_t));
    red_req->link_to(green_req);

    DMA::Channel c;
    LOG_DEBUG("DMA Start !");
    size_t start = GenericTimer::get_elapsed_time_in_ns();
    c.execute_requests(red_req);
    c.wait();  // Not needed, DMA is in background.
    size_t end = GenericTimer::get_elapsed_time_in_ns();
    LOG_DEBUG("DMA End.");
    LOG_DEBUG("Time: {} ns", end - start);

    delete red_req;
    delete green_req;
  }

  TaskManager* task_manager = new TaskManager;

  auto task1 = task_manager->create_task((const elf::Header*)&init);
  task_manager->wake_task(task1);

  auto task2 = task_manager->create_task((const elf::Header*)&init);
  task_manager->wake_task(task2);

  auto task3 = task_manager->create_task((const elf::Header*)&init);
  task_manager->wake_task(task3);

  //  Enter userspace
  task_manager->schedule();
  task_manager->get_current_task()->get_saved_state().memory->activate();
  jump_to_el0(task1->get_saved_state().regs.elr, (uintptr_t)task_manager->get_current_task()->get_saved_state().sp);
  LOG_CRITICAL("Not in user space");
  libk::halt();
}
