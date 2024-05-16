#pragma once

#include <cstddef>
#include <cstdint>

namespace GPIO {
static constexpr size_t NB_PINS = 58;

enum class Mode : uint8_t {
  // Values from BCM2835-ARM-Peripherals.pdf, page 92.
  INPUT = 0b000,
  OUTPUT = 0b001,
  ALT0 = 0b100,
  ALT1 = 0b101,
  ALT2 = 0b110,
  ALT3 = 0b111,
  ALT4 = 0b011,
  ALT5 = 0b010,
};

enum class PUD_Mode : uint8_t {
  // Values from BCM2835-ARM-Peripherals.pdf, page 101.
  Off = 0b00,
  PullDown = 0b01,
  PullUp = 0b10,
};

using CallBack = void (*)(size_t);

/** Set up GPIO */
void init();

/** Set the mode of a gpio pin. */
void set_mode(size_t gpio_pin, Mode target_mode);

/** Get the mode of a gpio pin. */
Mode get_mode(size_t gpio_pin);

/** Set/clear gpio pull up/down resistor of a gpio pin */
void set_pull_up_down(size_t gpio_pin, PUD_Mode target_pud_mode);

/** Read the value of a gpio pin */
bool read(size_t gpio_pin);

/** Sets a gpio pin to an OUTPUT and write the specified value to it. */
void write(size_t gpio_pin, bool on);

/** Checks if one of the event that is registered for the gpio pin @a gpio_pin has occurred.
 * If multiple event are registered for a pin, you can't detect which one it is. */
bool has_event(size_t gpio_pin);

/** Clear any event of gpio pin @a gpio_pin */
void clear_event(size_t gpio_pin);

/** Execute the function @a cb on event on pin @a gpio_pin. */
void set_event_callback(size_t gpio_pin, CallBack cb);

/** Remove the callback function for the gpio pin @a gpio_pin.
 * @a returns the old callback. */
CallBack remove_event_callback(size_t gpio_pin);

/** Set the Rising Edge event for the gpio pin @a gpio_pin. */
void set_rising_edge_detect(size_t gpio_pin, bool enable);
/** Set the Falling Edge event for the gpio pin @a gpio_pin. */
void set_falling_edge_detect(size_t gpio_pin, bool enable);

/** Set the Asynchronous Rising Edge event for the gpio pin @a gpio_pin. */
void set_rising_edge_async_detect(size_t gpio_pin, bool enable);
/** Set the Asynchronous Falling Edge event for the gpio pin @a gpio_pin. */
void set_falling_edge_async_detect(size_t gpio_pin, bool enable);

/** Set the Pin High event for the gpio pin @a gpio_pin. */
void set_pin_high_detect(size_t gpio_pin, bool enable);
/** Set the Pin Low event for the gpio pin @a gpio_pin. */
void set_low_detect(size_t gpio_pin, bool enable);

}  // namespace GPIO
