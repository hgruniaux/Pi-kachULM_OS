#include "hardware/framebuffer.hpp"

#include <libk/assert.hpp>
#include <libk/log.hpp>
#include "hardware/mailbox.hpp"

FrameBuffer& FrameBuffer::get() {
  static FrameBuffer framebuffer;
  return framebuffer;
}

[[maybe_unused]] static bool get_framebuffer_real_size(uint32_t& width, uint32_t& height) {
  struct GetFrameBufferSizeTagBuffer {
    uint32_t width;
    uint32_t height;
  };  // struct GetFrameBufferSizeTagBuffer

  using GetFrameBufferSizeTag = MailBox::PropertyTag<0x00040003, GetFrameBufferSizeTagBuffer>;

  MailBox::PropertyMessage<GetFrameBufferSizeTag> message = {};
  const bool success = MailBox::send_property(message);
  width = message.tag.buffer.width;
  height = message.tag.buffer.height;
  return success && MailBox::check_tag_status(message.tag.status);
}

bool FrameBuffer::init(uint32_t width, uint32_t height) {
  // Common to SetPhysicalSizeTag and SetVirtualSizeTag.
  struct SetFrameBufferSizeTagBuffer {
    uint32_t width;
    uint32_t height;
  };  // struct SetFrameBufferSizeTagBuffer

  struct SetVirtualOffsetTagBuffer {
    uint32_t x;
    uint32_t y;
  };  // struct SetVirtualOffsetTagBuffer

  union AllocateTagBuffer {
    uint32_t alignment;
    struct Response {
      uint32_t base_address;
      uint32_t size;
    } response;
  };  // struct AllocateTagBuffer

  using SetPhysicalSizeTag = MailBox::PropertyTag<0x00048003, SetFrameBufferSizeTagBuffer>;
  using SetVirtualSizeTag = MailBox::PropertyTag<0x00048004, SetFrameBufferSizeTagBuffer>;
  using SetVirtualOffsetTag = MailBox::PropertyTag<0x00048009, SetVirtualOffsetTagBuffer>;
  using SetPixelOrderTag = MailBox::PropertyTag<0x00048006, uint32_t>;
  using SetDepthTag = MailBox::PropertyTag<0x00048005, uint32_t>;
  using AllocateTag = MailBox::PropertyTag<0x00040001, AllocateTagBuffer>;
  using GetPitchTag = MailBox::PropertyTag<0x00040008, uint32_t>;

  struct alignas(16) PropertyMessage {
    uint32_t buffer_size = sizeof(PropertyMessage);
    uint32_t status = 0;
    SetPhysicalSizeTag set_physical_size_tag = {};
    SetVirtualSizeTag set_virtual_size_tag = {};
    SetVirtualOffsetTag set_virtual_offset_tag = {};
    SetDepthTag set_depth_tag = {};
    SetPixelOrderTag set_pixel_order_tag = {};
    AllocateTag allocate_tag = {};
    GetPitchTag get_pitch_tag = {};
    uint32_t end_tag = 0;
  };  // struct PropertyMessage

  // Allocate a virtual buffer double the size in case of double buffering.
#ifdef CONFIG_USE_DOUBLE_BUFFERING
  const uint32_t requested_virtual_height = height * 2;
#else
  const uint32_t requested_virtual_height = height;
#endif  // CONFIG_USE_DOUBLE_BUFFERING

  PropertyMessage message;
  message.set_physical_size_tag.status = 0;
  message.set_physical_size_tag.buffer.width = width;
  message.set_physical_size_tag.buffer.height = height;
  message.set_virtual_size_tag.buffer.width = width;
  message.set_virtual_size_tag.buffer.height = requested_virtual_height;
  message.set_virtual_offset_tag.buffer.x = 0;
  message.set_virtual_offset_tag.buffer.y = 0;
  message.set_depth_tag.buffer = 32;             // 32-bits per pixel (8-bits per component)
  message.set_pixel_order_tag.buffer = 0;        // BGR (so we have 0xRRGGBB, yep, this seems inverted but is not)
  message.allocate_tag.buffer.alignment = 4096;  // Which value to choose?
  message.get_pitch_tag.buffer = 0;
  const bool success = MailBox::send_property(message);
  if (!success)
    return false;

  // Read back the responses. The GPU may have changed some requested parameters.
  m_width = message.set_virtual_size_tag.buffer.width;
#ifdef CONFIG_USE_DOUBLE_BUFFERING
  m_height = message.set_virtual_size_tag.buffer.height / 2;
#else
  m_height = message.set_virtual_size_tag.buffer.height;
#endif  // CONFIG_USE_DOUBLE_BUFFERING
  m_pitch = message.get_pitch_tag.buffer / sizeof(m_buffer[0]);
  m_buffer_size = message.allocate_tag.buffer.response.size / sizeof(uint32_t);

  uint64_t buffer_address = message.allocate_tag.buffer.response.base_address;
  buffer_address &= 0x3FFFFFFF;  // convert GPU address to ARM address
  buffer_address = KernelMemory::get_virtual_vc_address(buffer_address);
  m_buffer = (uint32_t*)buffer_address;

  LOG_INFO("Framebuffer of size {}x{} allocated (requested {}x{})", m_width, m_height, width, height);

  // Dump some debugging information in case of something is not working as intended.
  LOG_DEBUG("Framebuffer at {:#x} of size {} bytes (pitch = {})", m_buffer, message.allocate_tag.buffer.response.size,
            m_pitch);
  LOG_DEBUG("Framebuffer pixel order: {} (0 = BGR, 1 = RGB)", message.set_pixel_order_tag.buffer);
  LOG_DEBUG("Framebuffer depth: {}", message.set_depth_tag.buffer);

  // Initially clear the framebuffer with black color. In case the VideoCore gives us
  // an uninitialized framebuffer.
  // This is called before double buffering initialization, so both the front and back
  // buffers are cleared.
  clear(0x00000000);

#ifdef CONFIG_USE_DOUBLE_BUFFERING
  // Display the screen 0.
  m_is_screen0 = true;
  set_virtual_offset(0, 0);
  // Switch to screen 1 (the back buffer)
  m_buffer_size = m_height * m_pitch;
  m_buffer += m_buffer_size;
#endif  // CONFIG_USE_DOUBLE_BUFFERING

  m_initialized = true;
  return true;
}

void FrameBuffer::clear(uint32_t color) {
  // Clear the current framebuffer.
  for (uint32_t i = 0; i < m_buffer_size; ++i) {
    m_buffer[i] = color;
  }
}

uint32_t FrameBuffer::get_pixel(uint32_t x, uint32_t y) const {
  KASSERT(x < m_width && y < m_height);
  return m_buffer[x + m_pitch * y];
}

void FrameBuffer::set_pixel(uint32_t x, uint32_t y, uint32_t color) {
  KASSERT(x < m_width && y < m_height);
  m_buffer[x + m_pitch * y] = color;
}

void FrameBuffer::present() {
#ifdef CONFIG_USE_DOUBLE_BUFFERING
  if (m_is_screen0) {
    // We are displaying screen0, switch to screen1.
    set_virtual_offset(0, m_height);
    // Swap back buffer <-> front buffer.
    m_buffer -= m_buffer_size;
  } else {
    set_virtual_offset(0, 0);
    m_buffer += m_buffer_size;
  }

  m_is_screen0 = !m_is_screen0;
#endif  // CONFIG_USE_DOUBLE_BUFFERING
}

bool FrameBuffer::set_virtual_offset(uint32_t x, uint32_t y) {
  struct SetVirtualOffsetTagBuffer {
    uint32_t x;
    uint32_t y;
  };  // struct SetVirtualOffsetTagBuffer

  using SetVirtualOffsetTag = MailBox::PropertyTag<0x00048009, SetVirtualOffsetTagBuffer>;

  MailBox::PropertyMessage<SetVirtualOffsetTag> message = {};
  message.tag.buffer.x = x;
  message.tag.buffer.y = y;
  const bool success = MailBox::send_property(message);
  return success && MailBox::check_tag_status(message.tag.status) && message.tag.buffer.x == x &&
         message.tag.buffer.y == y;
}
