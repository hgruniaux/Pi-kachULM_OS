#include "uart.hpp"
#include "gpio.hpp"
#include <libk/utils.hpp>
#include "mailbox.hpp"

/** UART0 Data Register (size: 32) */
static inline constexpr int UART_DR = 0x0;

/** UART0 Flag Register (size: 32) */
static inline constexpr int UART_FR = 0x18;

/** UART0 Integer Baud rate divisor (size: 32) */
static inline constexpr int UART_IBRD = 0x24;

/** UART0 Fractional Baud rate divisor (size: 32) */
static inline constexpr int UART_FBRD = 0x28;

/** UART0 Line Control Register (size: 32) */
static inline constexpr int UART_LCRH = 0x2c;

/** UART0 Control Register (size: 32) */
static inline constexpr int UART_CR = 0x30;

/** UART0 Interrupt FIFO Level Select Register (size: 32) */
// static inline constexpr int UART_IFLS = 0x34;

/** UART0 Interrupt Mask Set Clear Register (size: 32) */
// static inline constexpr int UART_IMSC = 0x38;

/** UART0 Raw Interrupt Status Register (size: 32) */
// static inline constexpr int UART_RIS = 0x3c;

/** UART0 Masked Interrupt Status Register (size: 32) */
// static inline constexpr int UART_MIS = 0x40;

/** UART0 Interrupt Clear Register (size: 32) */
// static inline constexpr int UART_ICR = 0x44;

/** UART0 DMA Control Register (size: 32) */
// static inline constexpr int UART_DMACR = 0x48;

/** UART0 Test Control Register (size: 32) */
// static inline constexpr int UART_ITCR = 0x80;

/** UART0 Integration Test Input Register (size: 32) */
// static inline constexpr int UART_ITIP = 0x84;

/** UART0 Integration Test Output Register (size: 32) */
// static inline constexpr int UART_ITOP = 0x88;

/** UART0 Test Data Register (size: 32) */
// static inline constexpr int UART_TDR = 0x8c;

uintptr_t find_uart_base(const UART::Id uart_id) {
  (void)uart_id;
  return 0xffff'2000'0000'0000 + 0x201000;
}

UART::UART(const Id uart_id, uint32_t baud_rate) : _uart_base(find_uart_base(uart_id)) {
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

  // IntegerPart = clock / (16 * baud rate)   <- Integer division
  // FractionalPart = 64 * (clock % (16 * baud rate)) / (16 * baud rate) = 4 * (clock % (16 * baud rate)) / (baud rate)
  const uint32_t integer_part = msg.tag.buffer.rate / (16 * baud_rate);
  const uint32_t fractional_part = 4 * (msg.tag.buffer.rate % (16 * baud_rate)) / baud_rate;
  libk::write32(_uart_base + UART_IBRD, integer_part);
  libk::write32(_uart_base + UART_FBRD, fractional_part);

  // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
  libk::write32(_uart_base + UART_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

  // Enable UART0, receive & transfer part of UART.
  libk::write32(_uart_base + UART_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void UART::write_one(char value) const {
  // Wait for UART to become ready to transmit.
  while ((libk::read32(_uart_base + UART_FR) & (1 << 5)) != 0) {
    libk::yield();
  }

  libk::write32(_uart_base + UART_DR, value);
}

char UART::read_one() const {
  // Wait for UART to have received something.
  while ((libk::read32(_uart_base + UART_FR) & (1 << 4)) != 0) {
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
