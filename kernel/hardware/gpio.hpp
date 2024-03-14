#pragma once

#include <cstdint>

namespace GPIO {
enum class Pin : uint8_t {
  BCM00 = 0,
  BCM01 = 1,
  BCM02 = 2,
  BCM03 = 3,
  BCM04 = 4,
  BCM05 = 5,
  BCM06 = 6,
  BCM07 = 7,
  BCM08 = 8,
  BCM09 = 9,
  BCM10 = 10,
  BCM11 = 11,
  BCM12 = 12,
  BCM13 = 13,
  BCM14 = 14,
  BCM15 = 15,
  BCM16 = 16,
  BCM17 = 17,
  BCM18 = 18,
  BCM19 = 19,
  BCM20 = 20,
  BCM21 = 21,
  BCM22 = 22,
  BCM23 = 23,
  BCM24 = 24,
  BCM25 = 25,
  BCM26 = 26,
  BCM27 = 27,
  BCM28 = 28,
  BCM29 = 29,
  BCM30 = 30,
  BCM31 = 31,
  BCM32 = 32,
  BCM33 = 33,
  BCM34 = 34,
  BCM35 = 35,
  BCM36 = 36,
  BCM37 = 37,
  BCM38 = 38,
  BCM39 = 39,
  BCM40 = 40,
  BCM41 = 41,
  BCM42 = 42,
  BCM43 = 43,
  BCM44 = 44,
  BCM45 = 45,
  BCM46 = 46,
  BCM47 = 47,
  BCM48 = 48,
  BCM49 = 49,
  BCM50 = 50,
  BCM51 = 51,
  BCM52 = 52,
  BCM53 = 53,
};

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

/** Set the mode of a gpio pin. */
void set_mode(Pin gpio_pin, Mode target_mode);

/** Get the mode of a gpio pin. */
Mode get_mode(Pin gpio_pin);

/** Set/clear gpio pull up/down resistor of a gpio pin */
void set_pull_up_down(Pin gpio_pin, PUD_Mode target_pud_mode);

/** Read the value of a gpio pin */
bool read(Pin gpio_pin);

/** Sets a gpio pin to an OUTPUT and write the specified value to it. */
void write(Pin gpio_pin, bool on);

}  // namespace GPIO
