#pragma once

#include <cstdint>

class Device {
 public:
  bool init();

  struct MemoryInfo {
    // FIXME: We store the size of the memory region in 32-bits because this data
    //        is retrieved from the ArmToVC property mailbox interface. Sadly,
    //        according to https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface,
    //        only 32-bits memory size is supported. So, only 4Gb max.
    //        BUT, some Raspberry Pi boards have more than 4Gb of memory, so there is
    //        a problem. No idea if it is intended or how it works on such boards.

    // The following structure must remain the same. It must be binary compatible to
    // what the mailbox property interface expect for tags "Get ARM memory" and "Get VC memory".
    // See https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface

    /** The base address of the memory region, in bytes. */
    uint32_t base_address;
    /** The size of the memory region, in bytes. */
    uint32_t size;
  };  // struct MemoryInfo

  /** @brief Gets the ARM memory info. */
  [[nodiscard]] MemoryInfo get_arm_memory_info() const { return m_arm_memory_info; }
  /** @brief Gets the VideoCore memory info. */
  [[nodiscard]] MemoryInfo get_vc_memory_info() const { return m_vc_memory_info; }

  [[nodiscard]] uint32_t get_board_model() const { return m_board_model; }
  [[nodiscard]] uint32_t get_board_revision() const { return m_board_revision; }
  [[nodiscard]] uint64_t get_board_serial() const { return m_board_serial; }

  /** @brief The two onboard supported LEDs. */
  enum class Led : uint8_t {
    /** @brief The onboard ACTIVITY LED. This is the one in green. */
    ACT = 42,
    /** @brief The onboard POWER LED. This is the one in red. */
    PWR = 130
  };  // enum class Led

  /**
   * @brief Turns on or off the requested onboard LED.
   *
   * Only the onboard ACTIVITY and POWER LEDs can be updated in Raspberry PI.
   * The ACTIVITY LED is the one in green, the POWER the one in red.
   *
   * Note, the different LEDs have normally a specific semantic. Avoid turning
   * them off and on without any good reasons other than their specified semantics.
   *
   * @paramn led The LED to update.
   * @param is_on True to turn on, false to turn off.
   * @return True if the LED was successfully updated. */
  bool set_led_status(Led led, bool is_on = true);

  /** @brief Gets the current board temperature in thousandths of a degree C.  */
  [[nodiscard]] uint32_t get_current_temp() const;
  /** @brief Gets the max board temperature allowed in thousandths of a degree C.  */
  [[nodiscard]] uint32_t get_max_temp() const { return m_max_temp; }

 private:
  // Cached values:
  MemoryInfo m_arm_memory_info, m_vc_memory_info;
  uint64_t m_board_serial;
  uint32_t m_board_model;
  uint32_t m_board_revision;
  uint32_t m_max_temp;
};  // class Device
