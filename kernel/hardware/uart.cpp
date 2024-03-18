#include "uart.hpp"
#include <cstddef>
#include <cstdint>
#include "../debug.hpp"
#include "gpio.hpp"
#include "mailbox.hpp"
#include "mmio.hpp"

namespace UART {

/** Base of all PL011 UART0 registers */
static constexpr int32_t UART0_BASE = 0x201000;

/** UART0 Data Register (size: 32) */
static constexpr int32_t UART0_DR = UART0_BASE + 0x0;

/** UART0 Flag Register (size: 32) */
static constexpr int32_t UART0_FR = UART0_BASE + 0x18;

/** UART0 Integer Baud rate divisor (size: 32) */
static constexpr int32_t UART0_IBRD = UART0_BASE + 0x24;

/** UART0 Fractional Baud rate divisor (size: 32) */
static constexpr int32_t UART0_FBRD = UART0_BASE + 0x28;

/** UART0 Line Control Register (size: 32) */
static constexpr int32_t UART0_LCRH = UART0_BASE + 0x2c;

/** UART0 Control Register (size: 32) */
static constexpr int32_t UART0_CR = UART0_BASE + 0x30;

/** UART0 Interrupt FIFO Level Select Register (size: 32) */
static constexpr int32_t UART0_IFLS = UART0_BASE + 0x34;

/** UART0 Interrupt Mask Set Clear Register (size: 32) */
static constexpr int32_t UART0_IMSC = UART0_BASE + 0x38;

/** UART0 Raw Interrupt Status Register (size: 32) */
static constexpr int32_t UART0_RIS = UART0_BASE + 0x3c;

/** UART0 Masked Interrupt Status Register (size: 32) */
static constexpr int32_t UART0_MIS = UART0_BASE + 0x40;

/** UART0 Interrupt Clear Register (size: 32) */
static constexpr int32_t UART0_ICR = UART0_BASE + 0x44;

/** UART0 DMA Control Register (size: 32) */
static constexpr int32_t UART0_DMACR = UART0_BASE + 0x48;

/** UART0 Test Control Register (size: 32) */
static constexpr int32_t UART0_ITCR = UART0_BASE + 0x80;

/** UART0 Integration Test Input Register (size: 32) */
static constexpr int32_t UART0_ITIP = UART0_BASE + 0x84;

/** UART0 Integration Test Output Register (size: 32) */
static constexpr int32_t UART0_ITOP = UART0_BASE + 0x88;

/** UART0 Test Data Register (size: 32) */
static constexpr int32_t UART0_TDR = UART0_BASE + 0x8c;

void init(uint32_t baud_rate) {
  // Set the UART Clock to 4MHz
  struct GetClockRateBuffer {
    uint32_t clock_id = 0;
    uint32_t rate = 0;
  };
  using GetClockRateTag = MailBox::PropertyTag<0x00030002, GetClockRateBuffer>;

  MailBox::PropertyMessage<GetClockRateTag> msg;
  msg.tag.buffer.clock_id = 0x000000002;
  send_property(msg);

  // Deactivate Pull Up/Down on pin 14 and 15
  GPIO::set_pull_up_down(GPIO::Pin::BCM14, GPIO::PUD_Mode::Off);
  GPIO::set_pull_up_down(GPIO::Pin::BCM15, GPIO::PUD_Mode::Off);

  // Set pin 14 and 15 to mode Alternate0
  GPIO::set_mode(GPIO::Pin::BCM14, GPIO::Mode::ALT0);
  GPIO::set_mode(GPIO::Pin::BCM15, GPIO::Mode::ALT0);

  // Clear pending interrupts.
  MMIO::write(UART0_ICR, 0x7FF);

  // IntegerPart = clock / (16 * baud rate)   <- Integer division
  // FractionalPart = 64 * (clock % (16 * baud rate)) / (16 * baud rate) = 4 * (clock % (16 * baud rate)) / (baud rate)
  const uint32_t integer_part = msg.tag.buffer.rate / (16 * baud_rate);
  const uint32_t fractional_part = 4 * (msg.tag.buffer.rate % (16 * baud_rate)) / baud_rate;
  MMIO::write(UART0_IBRD, integer_part);
  MMIO::write(UART0_FBRD, fractional_part);

  // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
  MMIO::write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

  // Mask all interrupts.
  MMIO::write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

  // Enable UART0, receive & transfer part of UART.
  MMIO::write(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void write_one(uint8_t value) {
  // Wait for UART to become ready to transmit.
  while ((MMIO::read(UART0_FR) & (1 << 5)) != 0) {
  }
  MMIO::write(UART0_DR, value);
}

uint8_t read_one() {
  // Wait for UART to have received something.
  while ((MMIO::read(UART0_FR) & (1 << 4)) != 0) {
  }

  return MMIO::read(UART0_DR);
}

void write(const uint8_t* buffer, size_t buffer_length) {
  for (size_t i = 0; i < buffer_length; i++)
    write_one(buffer[i]);
}

void read(uint8_t* buffer, size_t buffer_length) {
  for (size_t i = 0; i < buffer_length; i++)
    buffer[i] = read_one();
}

void puts(const char* buffer) {
  const char* it = buffer;
  while (*it != '\0')
    write_one((uint8_t)*it++);
}
}  // namespace UART
