#include <cstdint>

#include "gpio.hpp"
#include "miniuart.hpp"
#include "mmio.hpp"

/** Auxiliary Interrupt status (size: 3) */
static constexpr int32_t AUX_IRQ = 0x215000;

/** Auxiliary enables (size: 3) */
static constexpr int32_t AUX_ENABLES = 0x215004;

/** Mini UART I/O Data (size: 8) */
static constexpr int32_t AUX_MU_IO_REG = 0x215040;

/** Mini UART Interrupt Enable (size: 8) */
static constexpr int32_t AUX_MU_IER_REG = 0x215044;

/** Mini UART Interrupt Identify (size: 8) */
static constexpr int32_t AUX_MU_IIR_REG = 0x215048;

/** Mini UART Line Control (size: 8) */
static constexpr int32_t AUX_MU_LCR_REG = 0x21504C;

/** Mini UART Modem Control (size: 8) */
static constexpr int32_t AUX_MU_MCR_REG = 0x215050;

/** Mini UART Line Status (size: 8) */
static constexpr int32_t AUX_MU_LSR_REG = 0x215054;

/** Mini UART Modem Status (size: 8) */
static constexpr int32_t AUX_MU_MSR_REG = 0x215058;

/** Mini UART Scratch (size: 8) */
static constexpr int32_t AUX_MU_SCRATCH = 0x21505C;

/** Mini UART Extra Control (size: 8) */
static constexpr int32_t AUX_MU_CNTL_REG = 0x215060;

/** Mini UART Extra Status (size: 32) */
static constexpr int32_t AUX_MU_STAT_REG = 0x215064;

/** Mini UART Baudrate (size: 16) */
static constexpr int32_t AUX_MU_BAUD_REG = 0x215068;

//
// void uart_send ( char c )
//{
//	while(1) {
//		if(get32(AUX_MU_LSR_REG)&0x20)
//			break;
//	}
//	put32(AUX_MU_IO_REG,c);
// }
//
// char uart_recv ( void )
//{
//	while(1) {
//		if(get32(AUX_MU_LSR_REG)&0x01)
//			break;
//	}
//	return(get32(AUX_MU_IO_REG)&0xFF);
// }
//
// void uart_send_string(char* str)
//{
//	for (int i = 0; str[i] != '\0'; i ++) {
//		uart_send((char)str[i]);
//	}
// }

void MINI_UART::init(uint32_t baudrate) {
  // We deactivate Pull Up/Down fot the pins 14 and 15
  GPIO::set_pull_up_down(GPIO::Pin::BCM14, GPIO::PUD_Mode::Off);
  GPIO::set_pull_up_down(GPIO::Pin::BCM15, GPIO::PUD_Mode::Off);

  // Pin 14 and 15 must be in mode Alt5
  GPIO::set_mode(GPIO::Pin::BCM14, GPIO::Mode::ALT5);
  GPIO::set_mode(GPIO::Pin::BCM15, GPIO::Mode::ALT5);

  // Enable Mini UART (this also enables access to its registers)
  MMIO::write(AUX_ENABLES, 1);

  // Disable auto flow control and disable receiver and transmitter (for now)
  MMIO::write(AUX_MU_CNTL_REG, 0);

  // Disable receive and transmit interrupts
  MMIO::write(AUX_MU_IER_REG, 0);

  // Enable 8 bit mode
  MMIO::write(AUX_MU_LCR_REG, 3);

  // Set RTS line to be always high
  MMIO::write(AUX_MU_MCR_REG, 0);

  // Set baud rate
#ifdef
  MMIO::write(AUX_MU_BAUD_REG, TODO);

  // Finally, enable transmitter and receiver
  MMIO::write(AUX_MU_CNTL_REG, 3);
}
