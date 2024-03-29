#include "device.hpp"
#include "../debug.hpp"
#include "mailbox.hpp"

bool Device::init() {
  struct GetBoardSerialTagBuffer {
    uint32_t serial_data[2];
  };

  using GetGetBoardModelTag = MailBox::PropertyTag<0x00010001, uint32_t>;
  using GetGetBoardRevisionTag = MailBox::PropertyTag<0x00010002, uint32_t>;
  using GetGetBoardSerialTag = MailBox::PropertyTag<0x00010004, GetBoardSerialTagBuffer>;
  using GetARMMemoryTag = MailBox::PropertyTag<0x00010005, MemoryInfo>;
  using GetVCMemoryTag = MailBox::PropertyTag<0x00010006, MemoryInfo>;

  struct GetMaxTemperatureTagBuffer {
    uint32_t id = 0;
    uint32_t value = 0;
  };

  using GetMaxTemperatureTag = MailBox::PropertyTag<0x0003000a, GetMaxTemperatureTagBuffer>;

  struct alignas(16) PropertyMessage {
    uint32_t buffer_size = sizeof(PropertyMessage);
    uint32_t status = 0;
    volatile GetGetBoardModelTag board_model_tag = {};        // Can be modified by the GPU
    volatile GetGetBoardRevisionTag board_revision_tag = {};  // Can be modified by the GPU
    volatile GetGetBoardSerialTag board_serial_tag = {};      // Can be modified by the GPU
    volatile GetARMMemoryTag arm_memory_tag = {};             // Can be modified by the GPU
    volatile GetVCMemoryTag vc_memory_tag = {};               // Can be modified by the GPU
    volatile GetMaxTemperatureTag max_temp_tag = {};          // Can be modified by the GPU
    uint32_t end_tag = 0;
  };

  PropertyMessage message;
  const bool success = MailBox::send_property(message);

  // Cache values
  m_max_temp = message.max_temp_tag.buffer.value;

  m_board_model = message.board_model_tag.buffer;
  m_board_revision = message.board_revision_tag.buffer;

  m_board_serial = (uint64_t)(message.board_serial_tag.buffer.serial_data[1]) << 32 |
                   (uint64_t)message.board_serial_tag.buffer.serial_data[0];

  m_arm_memory_info.size = message.arm_memory_tag.buffer.size;
  m_arm_memory_info.base_address = message.arm_memory_tag.buffer.base_address;

  m_vc_memory_info.size = message.vc_memory_tag.buffer.size;
  m_vc_memory_info.base_address = message.vc_memory_tag.buffer.base_address;

  return success;
}

bool Device::set_led_status(Device::Led led, bool on) {
  KASSERT(led == Led::ACT || led == Led::PWR);

  struct SetLedStatusTagBuffer {
    // Either 42 (ACT) or 130 (PWR)
    uint32_t pin;
    // 0 or 1
    uint32_t status;
  };  // struct SetLedStatusTagBuffer

  using SetLedStatusTag = MailBox::PropertyTag<0x00038041, SetLedStatusTagBuffer>;

  const uint32_t status = on ? 1 : 0;
  MailBox::PropertyMessage<SetLedStatusTag> message;
  message.tag.buffer.pin = (uint32_t)(led);
  message.tag.buffer.status = status;
  const bool success = MailBox::send_property(message);
  return success && (message.tag.buffer.status == status);
}

uint32_t Device::get_current_temp() const {
  struct GetTempTagBuffer {
    // Always 0.
    uint32_t id = 0;
    uint32_t value = 0;
  };  // struct GetTempTagBuffer

  using GetTempTag = MailBox::PropertyTag<0x00030006, GetTempTagBuffer>;

  MailBox::PropertyMessage<GetTempTag> message;
  const bool success = MailBox::send_property(message);
  return success ? message.tag.buffer.value : 0;
}

bool Device::set_power_state(uint32_t device_id, bool on, bool wait) {
  struct SetPowerStateTagBuffer {
    uint32_t device_id = 0;
    uint32_t state = 0;
  };  // struct SetPowerStateTagBuffer

  using SetPowerStateTag = MailBox::PropertyTag<0x00028001, SetPowerStateTagBuffer>;

  MailBox::PropertyMessage<SetPowerStateTag> message;
  message.tag.buffer.device_id = device_id;
  message.tag.buffer.state = (uint32_t)on | ((uint32_t)wait << 1);
  const bool success = MailBox::send_property(message);
  return success && ((message.tag.buffer.state & 0x1) == (uint32_t)on)  // check if the device is in the expected state
         && ((message.tag.buffer.state & 0x2) != 0);                    // check if the device exists
}

bool Device::set_turbo(bool on) {
  struct SetTurboTagBuffer {
    // Always 0.
    uint32_t id = 0;
    // 1 to enable Turbo, 0 otherwise.
    uint32_t level = 0;
  };  // struct SetTurboTagBuffer

  using SetTurboTag = MailBox::PropertyTag<0x00038009, SetTurboTagBuffer>;

  const uint32_t level = on ? 1 : 0;
  MailBox::PropertyMessage<SetTurboTag> message;
  message.tag.buffer.level = level;
  const bool success = MailBox::send_property(message);
  return success && (message.tag.buffer.level == level);
}
