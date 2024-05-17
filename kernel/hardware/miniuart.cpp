#include "hardware/miniuart.hpp"

#include <libk/utils.hpp>
#include "device.hpp"
#include "hardware/gpio.hpp"
#include "kernel_dt.hpp"

/** Auxiliary Interrupt status (size: 3) */
// static constexpr int32_t AUX_IRQ = 0x00;

/** Auxiliary enables (size: 3) */
static constexpr int32_t AUX_ENABLES = 0x04;

/** Mini UART I/O Data (size: 8) */
static constexpr int32_t AUX_MU_IO_REG = 0x40;

/** Mini UART Interrupt Enable (size: 8) */
static constexpr int32_t AUX_MU_IER_REG = 0x44;

/** Mini UART Interrupt Identify (size: 8) */
// static constexpr int32_t AUX_MU_IIR_REG = 0x48;

/** Mini UART Line Control (size: 8) */
static constexpr int32_t AUX_MU_LCR_REG = 0x4C;

/** Mini UART Modem Control (size: 8) */
static constexpr int32_t AUX_MU_MCR_REG = 0x50;

/** Mini UART Line Status (size: 8) */
static constexpr int32_t AUX_MU_LSR_REG = 0x54;

/** Mini UART Extra Control (size: 8) */
static constexpr int32_t AUX_MU_CNTL_REG = 0x60;

/** Mini UART Baud rate (size: 16) */
static constexpr int32_t AUX_MU_BAUD_REG = 0x68;

MiniUART::MiniUART(uint32_t baud_rate) : _mini_uart_base(KernelDT::force_get_device_address("aux")) {
  // We deactivate Pull Up/Down fot the pins 14 and 15
  GPIO::set_pull_up_down(14, GPIO::PUD_Mode::Off);
  GPIO::set_pull_up_down(15, GPIO::PUD_Mode::Off);

  // Pin 14 and 15 must be in mode Alt5
  GPIO::set_mode(14, GPIO::Mode::ALT5);
  GPIO::set_mode(15, GPIO::Mode::ALT5);

  // Enable Mini UART (this also enables access to its registers)
  libk::write32(_mini_uart_base + AUX_ENABLES, 1);

  // Disable auto flow control and disable receiver and transmitter (for now)
  libk::write32(_mini_uart_base + AUX_MU_CNTL_REG, 0);

  // Disable receive and transmit interrupts
  libk::write32(_mini_uart_base + AUX_MU_IER_REG, 0);

  // Enable 8 bit mode
  libk::write32(_mini_uart_base + AUX_MU_LCR_REG, 3);

  // Set RTS line to be always high
  libk::write32(_mini_uart_base + AUX_MU_MCR_REG, 0);

  const uint32_t core_clock = Device::get_clock_rate(Device::CORE);
  libk::write32(_mini_uart_base + AUX_MU_BAUD_REG, core_clock / ((uint64_t)8 * baud_rate) - 1);

  // Finally, enable transmitter and receiver
  libk::write32(_mini_uart_base + AUX_MU_CNTL_REG, 3);
}

void MiniUART::write_one(char value) const {
  while ((libk::read32(_mini_uart_base + AUX_MU_LSR_REG) & 0x20) == 0) {
    asm("yield");
  }

  libk::write32(_mini_uart_base + AUX_MU_IO_REG, value);
}

char MiniUART::read_one() const {
  while ((libk::read32(_mini_uart_base + AUX_MU_LSR_REG) & 0x01) == 0) {
    asm("yield");
  }

  return (libk::read32(_mini_uart_base + AUX_MU_IO_REG) & 0xFF);
}

void MiniUART::puts(const char* buffer) const {
  while (*buffer != '\0') {
    write_one(*(buffer++));
  }
}

void MiniUART::write(const char* buffer, size_t buffer_length) const {
  for (size_t i = 0; i < buffer_length; ++i) {
    write_one(buffer[i]);
  }
}

void MiniUART::read(char* buffer, size_t buffer_length) const {
  for (size_t i = 0; i < buffer_length; ++i) {
    buffer[i] = read_one();
  }
}

void MiniUART::writeln(const char* buffer, size_t buffer_length) {
  MiniUART::write(buffer, buffer_length);
  MiniUART::puts("\r\n");
}
