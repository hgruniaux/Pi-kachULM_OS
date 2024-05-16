#include "hardware/gpio.hpp"

#include <libk/utils.hpp>
#include "hardware/kernel_dt.hpp"

// Offset from the peripheral base, taken from BCM2835-ARM-Peripherals.pdf, page 90-91

static constexpr int32_t GPFSEL = 0x00;  // GPIO Function Select Base

// GPFSEL + 0  = 0x00, GPIO Function Select 0
// GPFSEL + 4  = 0x04, GPIO Function Select 1
// GPFSEL + 8  = 0x08, GPIO Function Select 2
// GPFSEL + 12 = 0x0C, GPIO Function Select 3
// GPFSEL + 16 = 0x10, GPIO Function Select 4
// GPFSEL + 20 = 0x14, GPIO Function Select 5

static constexpr int32_t GPSET = 0x1C;  // GPIO Pin Output Set Base

//  GPSET + 0 = 0x1C, GPIO Pin Output Set 0
//  GPSET + 4 = 0x20, GPIO Pin Output Set 1

static constexpr int32_t GPCLR = 0x28;  // GPIO Pin Output Clear Base

//  GPCLR + 0 = 0x28, GPIO Pin Output Clear 1
//  GPCLR + 4 = 0x2C, GPIO Pin Output Clear 1

static constexpr int32_t GPLEV = 0x34;  // GPIO Pin Level Base

//  GPLEV + 0 = 0x34, GPIO Pin Level 0
//  GPLEV + 4 = 0x38, GPIO Pin Level 1

// static constexpr int32_t GPEDS = 0x40;  // GPIO Pin Event Detect Status Base
//  GPEDS + 0 = 0x40, GPIO Pin Event Detect Status 0
//  GPEDS + 4 = 0x44, GPIO Pin Event Detect Status 1

// static constexpr int32_t GPREN = 0x4C;  // GPIO Pin Rising Edge Detect Enable Base
//  GPREN + 0 = 0x4C, GPIO Pin Rising Edge Detect Enable 0
//  GPREN + 4 = 0x50, GPIO Pin Rising Edge Detect Enable 1

// static constexpr int32_t GPFEN = 0x58;  // GPIO Pin Falling Edge Detect Enable Base
//  GPFEN + 0 = 0x58, GPIO Pin Falling Edge Detect Enable 0
//  GPFEN + 4 = 0x5C, GPIO Pin Falling Edge Detect Enable 1

// static constexpr int32_t GPHEN = 0x64;  // GPIO Pin High Detect Enable Base
//  GPHEN + 0 = 0x64, GPIO Pin High Detect Enable 0
//  GPHEN + 4 = 0x68, GPIO Pin High Detect Enable 1

// static constexpr int32_t GPLEN = 0x70;  // GPIO Pin Low Detect Enable Base
//  GPLEN + 0 = 0x70, GPIO Pin Low Detect Enable 0
//  GPLEN + 4 = 0x74, GPIO Pin Low Detect Enable 1

// static constexpr int32_t GPAREN = 0x7C;  // GPIO Pin Async. Rising Edge Detect
//  GPAREN + 0 = 0x7C, GPIO Pin Async. Rising Edge Detect 0
//  GPAREN + 4 = 0x80, GPIO Pin Async. Rising Edge Detect 1

// static constexpr int32_t GPAFEN = 0x88;  // GPIO Pin Async. Falling Edge Detect
//  GPAFEN + 0 = 0x88, GPIO Pin Async. Falling Edge Detect 0
//  GPAFEN + 4 = 0x8C, GPIO Pin Async. Falling Edge Detect 1

static constexpr int32_t GPPUD = 0x94;  // GPIO Pin Pull-up/down Enable

static constexpr int32_t GPPUDCLK = 0x98;  // GPIO Pin Pull-up/down Enable Clock

//  GPPUDCLK + 0 = 0x98, GPIO Pin Pull-up/down Enable Clock 0
//  GPPUDCLK + 4 = 0x9C, GPIO Pin Pull-up/down Enable Clock 1

static uintptr_t gpio_base;

void GPIO::init() {
  gpio_base = KernelDT::force_get_device_address("gpio");
}

void GPIO::set_mode(GPIO::Pin gpio_pin, GPIO::Mode target_mode) {
  const auto target_mode_int = static_cast<uint8_t>(target_mode);
  const auto gpio_pin_int = static_cast<uint8_t>(gpio_pin);

  // We will use GPFSEL{reg} to set the pin mode
  const uint8_t reg = gpio_pin_int / 10;

  // We use 3 bits per pin in each register to specify their modes
  const uint8_t shift = (gpio_pin_int % 10) * 3;

  const uint32_t old_mode = libk::read32(gpio_base + (GPFSEL + 4 * reg));

  // If we already are in the good mode, do nothing.
  if (target_mode_int != ((old_mode >> shift) & 7)) {
    //                               Our pin
    //                                  v
    // new_mode = (old_mode & 111...111000111...111) | (target_mode << shift)
    const uint32_t new_mode = (old_mode & ~(7 << shift)) | (target_mode_int << shift);
    libk::write32(gpio_base + GPFSEL + 4 * reg, new_mode);
  }
}

GPIO::Mode GPIO::get_mode(GPIO::Pin gpio_pin) {
  const auto gpio_pin_int = static_cast<uint8_t>(gpio_pin);

  // We will use GPFSEL{reg} to set the pin mode
  const uint8_t reg = gpio_pin_int / 10;

  // We use 3 bits per pin in each register to specify their modes
  const uint8_t shift = (gpio_pin_int % 10) * 3;

  const uint32_t mode = libk::read32(gpio_base + (GPFSEL + 4 * reg));

  return static_cast<GPIO::Mode>((mode >> shift) & 7);
}

void GPIO::set_pull_up_down(GPIO::Pin gpio_pin, GPIO::PUD_Mode target_pud_mode) {
  const auto gpio_pin_int = static_cast<uint8_t>(gpio_pin);

  /*
   * 1. Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down or neither to remove the current
        Pull-up/down)
   * 2. Wait 150 cycles – this provides the required set-up time for the control signal
   * 3. Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to modify – NOTE only the pads
        which receive a clock will be modified, all others will retain their previous state.
   * 4. Wait 150 cycles – this provides the required hold time for the control signal
   * 5. Write to GPPUD to remove the control signal
   * 6. Write to GPPUDCLK0/1 to remove the clock
   */

  const auto control_signal = static_cast<uint8_t>(target_pud_mode);
  libk::write32(gpio_base + GPPUD, control_signal);

  libk::wait_cycles(150);

  const uint8_t reg = gpio_pin_int / 32;
  const uint8_t shift = gpio_pin_int % 32;
  libk::write32(gpio_base + GPPUDCLK + 4 * reg, 1ul << shift);

  libk::wait_cycles(150);

  libk::write32(gpio_base + GPPUD, 0);
  libk::write32(gpio_base + GPPUDCLK + 4 * reg, 0);
}

bool GPIO::read(GPIO::Pin gpio_pin) {
  const auto gpio_pin_int = static_cast<uint8_t>(gpio_pin);

  const uint8_t reg = gpio_pin_int / 32;
  const uint8_t shift = gpio_pin_int % 32;

  const uint32_t val = libk::read32(gpio_base + (GPLEV + 4 * reg));

  return ((val >> shift) & 0x1) != 0u;
}

void GPIO::write(GPIO::Pin gpio_pin, bool on) {
  const auto gpio_pin_int = static_cast<uint8_t>(gpio_pin);
  set_mode(gpio_pin, Mode::OUTPUT);

  const uint8_t reg = gpio_pin_int / 32;
  const uint8_t shift = gpio_pin_int % 32;

  if (on) {
    libk::write32(gpio_base + GPSET + 4 * reg, 1ul << shift);
  } else {
    libk::write32(gpio_base + GPCLR + 4 * reg, 1ul << shift);
  }
}
