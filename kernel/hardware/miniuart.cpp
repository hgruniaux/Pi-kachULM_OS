#include "miniuart.hpp"
#include <libk/utils.hpp>
#include "../boot/mmu_utils.hpp"
#include "gpio.hpp"

namespace MiniUART {
/** Auxiliary Interrupt status (size: 3) */
// static constexpr int32_t AUX_IRQ = 0x215000;

/** Auxiliary enables (size: 3) */
static constexpr int32_t AUX_ENABLES = 0x215004;

/** Mini UART I/O Data (size: 8) */
static constexpr int32_t AUX_MU_IO_REG = 0x215040;

/** Mini UART Interrupt Enable (size: 8) */
static constexpr int32_t AUX_MU_IER_REG = 0x215044;

/** Mini UART Interrupt Identify (size: 8) */
// static constexpr int32_t AUX_MU_IIR_REG = 0x215048;

/** Mini UART Line Control (size: 8) */
static constexpr int32_t AUX_MU_LCR_REG = 0x21504C;

/** Mini UART Modem Control (size: 8) */
static constexpr int32_t AUX_MU_MCR_REG = 0x215050;

/** Mini UART Line Status (size: 8) */
static constexpr int32_t AUX_MU_LSR_REG = 0x215054;

/** Mini UART Extra Control (size: 8) */
static constexpr int32_t AUX_MU_CNTL_REG = 0x215060;

/** Mini UART Baud rate (size: 16) */
static constexpr int32_t AUX_MU_BAUD_REG = 0x215068;

static constexpr uint64_t RPI3_CORE_CLOCK = 250'000'000;

static constexpr uint64_t RPI4_CORE_CLOCK = 200'000'000;

void init(uint32_t baud_rate) {
  // We deactivate Pull Up/Down fot the pins 14 and 15
  GPIO::set_pull_up_down(GPIO::Pin::BCM14, GPIO::PUD_Mode::Off);
  GPIO::set_pull_up_down(GPIO::Pin::BCM15, GPIO::PUD_Mode::Off);
  //  TODO : FixThis!
  // Pin 14 and 15 must be in mode Alt5
  GPIO::set_mode(GPIO::Pin::BCM14, GPIO::Mode::ALT5);
  GPIO::set_mode(GPIO::Pin::BCM15, GPIO::Mode::ALT5);

  // Enable Mini UART (this also enables access to its registers)
  libk::write32(DEVICE_MEMORY + AUX_ENABLES, 1);

  // Disable auto flow control and disable receiver and transmitter (for now)
  libk::write32(DEVICE_MEMORY + AUX_MU_CNTL_REG, 0);

  // Disable receive and transmit interrupts
  libk::write32(DEVICE_MEMORY + AUX_MU_IER_REG, 0);

  // Enable 8 bit mode
  libk::write32(DEVICE_MEMORY + AUX_MU_LCR_REG, 3);

  // Set RTS line to be always high
  libk::write32(DEVICE_MEMORY + AUX_MU_MCR_REG, 0);

  //  TODO : FixThis!
  //  switch (MMIO::device) {
  //    case MMIO::DeviceType::RaspberryPi3:
  libk::write32(DEVICE_MEMORY + AUX_MU_BAUD_REG, RPI3_CORE_CLOCK / ((uint64_t)8 * baud_rate) - 1);
  //      break;
  //    case MMIO::DeviceType::RaspberryPi4:
  //      libk::write32(DEVICE_MEMORY + AUX_MU_BAUD_REG, RPI4_CORE_CLOCK / ((uint64_t)8 * baud_rate) - 1);
  //      break;
  //  }

  // Finally, enable transmitter and receiver
  libk::write32(DEVICE_MEMORY + AUX_MU_CNTL_REG, 3);
}

void write_one(uint8_t value) {
  const uintptr_t base =  DEVICE_MEMORY;

  while ((libk::read32(base + AUX_MU_LSR_REG) & 0x20) == 0) {
    asm("yield");
  }

  libk::write32(base + AUX_MU_IO_REG, value);
}

uint8_t read_one() {
  while ((libk::read32(DEVICE_MEMORY + AUX_MU_LSR_REG) & 0x01) == 0) {
    asm("yield");
  }

  return (libk::read32(DEVICE_MEMORY + AUX_MU_IO_REG) & 0xFF);
}

void puts(const char* buffer) {
  while (*buffer != '\0') {
    write_one(*(buffer++));
  }
}

void write(const uint8_t* buffer, size_t buffer_length) {
  for (size_t i = 0; i < buffer_length; ++i) {
    write_one(buffer[i]);
  }
}

void read(uint8_t* buffer, size_t buffer_length) {
  for (size_t i = 0; i < buffer_length; ++i) {
    buffer[i] = read_one();
  }
}

};  // namespace MiniUART
