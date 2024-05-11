#pragma once

#include <libk/string_view.hpp>

namespace Device {
[[nodiscard]] bool init();

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
 * @param on True to turn on, false to turn off.
 * @return True if the LED was successfully updated. */
bool set_led_status(Led led, bool on);

/** @brief Gets the current board temperature in thousandths of a degree C.  */
[[nodiscard]] uint32_t get_current_temp();
/** @brief Gets the max board temperature allowed in thousandths of a degree C.  */
[[nodiscard]] uint32_t get_max_temp();

/** @brief Turn on or off a given peripherical device.
 *
 * To know what device_id to use, please see:
 * https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
 *
 * @param on True to turn on, false to turn off.
 * @param wait If true, wait for the power to become stable for the device.
 * @return True in case of success.
 */
bool set_power_state(uint32_t device_id, bool on, bool wait = true);

/** @brief Enables or disables the TURBO mode.
 *
 * When the TURBO mode is enabled, the clock of the GPU will be set
 * to the maximum.
 *
 * @param on True to enable the TURBO mode, false to disable it.
 * @return True in case of success.
 */
bool set_turbo(bool on);

enum ClockId {
  EMMC = 0x000000001,
  UART = 0x000000002,
  ARM = 0x000000003,
  CORE = 0x000000004,
  V3D = 0x000000005,
  H264 = 0x000000006,
  ISP = 0x000000007,
  SDRAM = 0x000000008,
  PIXEL = 0x000000009,
  PWM = 0x00000000a,
  HEVC = 0x00000000b,
  EMMC2 = 0x00000000c,
  M2MC = 0x00000000d,
  PIXEL_BVB = 0x00000000e,
};

uint32_t get_clock_rate(ClockId id);
};  // namespace Device
