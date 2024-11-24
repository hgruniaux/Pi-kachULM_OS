#include "hardware/uart.hpp"

#include <libk/utils.hpp>
#include "device.hpp"
#include "hardware/gpio.hpp"
#include "hardware/mailbox.hpp"
#include "kernel_dt.hpp"

/** UART Data Register (size: 32) */
static inline constexpr int UART_DR = 0x0;

/** UART Flag Register (size: 32) */
static inline constexpr int UART_FR = 0x18;

/** UART Integer Baud rate divisor (size: 32) */
static inline constexpr int UART_IBRD = 0x24;

/** UART Fractional Baud rate divisor (size: 32) */
static inline constexpr int UART_FBRD = 0x28;

/** UART Line Control Register (size: 32) */
static inline constexpr int UART_LCRH = 0x2c;

/** UART Control Register (size: 32) */
static inline constexpr int UART_CR = 0x30;

/** UART Interrupt FIFO Level Select Register (size: 32) */
// static inline constexpr int UART_IFLS = 0x34;

/** UART Interrupt Mask Set Clear Register (size: 32) */
static inline constexpr int UART_IMSC = 0x38;

/** UART Raw Interrupt Status Register (size: 32) */
// static inline constexpr int UART_RIS = 0x3c;

/** UART Masked Interrupt Status Register (size: 32) */
// static inline constexpr int UART_MIS = 0x40;

/** UART Interrupt Clear Register (size: 32) */
// static inline constexpr int UART_ICR = 0x44;

/** UART DMA Control Register (size: 32) */
// static inline constexpr int UART_DMACR = 0x48;

/** UART Test Control Register (size: 32) */
// static inline constexpr int UART_ITCR = 0x80;

/** UART Integration Test Input Register (size: 32) */
// static inline constexpr int UART_ITIP = 0x84;

/** UART Integration Test Output Register (size: 32) */
// static inline constexpr int UART_ITOP = 0x88;

/** UART Test Data Register (size: 32) */
// static inline constexpr int UART_TDR = 0x8c;

UART::UART(uint32_t baud_rate, libk::StringView name, bool enabling_irqs)
    : _uart_base(KernelDT::force_get_device_address(name)) {
  // We don't yet support IRQs for UART1 (UART1 is a bit different from the others).
  KASSERT(!enabling_irqs || name != "uart1");

  // Get the UART Clock
  uint32_t uart_clock = Device::get_clock_rate(Device::UART);

  GPIO::set_pull_up_down(14, GPIO::PUD_Mode::Off);
  GPIO::set_pull_up_down(15, GPIO::PUD_Mode::Off);

  // Set pin 14 and 15 to mode Alternate0
  GPIO::set_mode(14, GPIO::Mode::ALT0);
  GPIO::set_mode(15, GPIO::Mode::ALT0);

  // IntegerPart = clock / (16 * baud rate)   <- Integer division
  // FractionalPart = 64 * (clock % (16 * baud rate)) / (16 * baud rate) = 4 * (clock % (16 * baud rate)) / (baud
  // rate)
  const uint32_t integer_part = uart_clock / (16 * baud_rate);
  const uint32_t fractional_part = 4 * (uart_clock % (16 * baud_rate)) / baud_rate;
  libk::write32(_uart_base + UART_IBRD, integer_part);
  libk::write32(_uart_base + UART_FBRD, fractional_part);

  // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
  libk::write32(_uart_base + UART_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

  // Enable UART0, receive & transfer part of UART.
  libk::write32(_uart_base + UART_CR, (1 << 0) | (1 << 8) | (1 << 9));

  if (enabling_irqs) {
    // Enable IRQs
    libk::write32(_uart_base + UART_IMSC, (1 << 1) | (1 << 4));
  }
}

bool UART::is_fifo_empty() const {
  return (libk::read32(_uart_base + UART_FR) & (1 << 4)) != 0;
}

bool UART::is_fifo_full() const {
  return (libk::read32(_uart_base + UART_FR) & (1 << 5)) != 0;
}

void UART::write_one(char value) const {
  // Wait for UART to become ready to transmit.
  while (is_fifo_full()) {
    libk::yield();
  }

  libk::write32(_uart_base + UART_DR, value);
}

char UART::read_one() const {
  // Wait for UART to have received something.
  while (is_fifo_empty()) {
    libk::yield();
  }

  return libk::read32(_uart_base + UART_DR);
}

void UART::write(const char* buffer, size_t buffer_length) const {
  for (size_t i = 0; i < buffer_length; i++) {
    write_one(buffer[i]);
  }
}

void UART::read(char* buffer, size_t buffer_length) const {
  for (size_t i = 0; i < buffer_length; i++) {
    buffer[i] = read_one();
  }
}

void UART::puts(const char* buffer) const {
  const char* it = buffer;
  while (*it != '\0') {
    write_one((uint8_t)*it++);
  }
}

void UART::writeln(const char* buffer, size_t buffer_length) {
  write(buffer, buffer_length);
  puts("\r\n");
}
