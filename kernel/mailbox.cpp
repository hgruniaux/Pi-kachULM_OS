#include "mailbox.hpp"
#include "mmio.hpp"

namespace MailBox {
/** Base address for mailbox MMIO registers. */
static constexpr uint32_t BASE = 0xB880;

/** Base address for mailbox 0 registers. */
static constexpr uint32_t BASE0 = BASE + 0x00;
/** The read/write register address for the mailbox 0. */
static constexpr uint32_t MBOX0_RW = BASE0 + 0x00;
/** The status register address for the mailbox 0. */
static constexpr uint32_t MBOX0_STATUS = BASE0 + 0x18;

/** Base address for mailbox 1 registers. */
static constexpr uint32_t BASE1 = BASE + 0x20;
/** The read/write register address for the mailbox 0. */
static constexpr uint32_t MBOX1_RW = BASE1 + 0x00;
/** The status register address for the mailbox 0. */
//static constexpr uint32_t MBOX1_STATUS = BASE1 + 0x18; // unused

static constexpr uint32_t CHANNEL_BITS = 4;
static constexpr uint32_t CHANNEL_MASK = (1 << CHANNEL_BITS) - 1;
static constexpr uint32_t STATUS_EMPTY_MASK = (1 << 30);
static constexpr uint32_t STATUS_FULL_MASK = (1 << 31);

/** Checks if given mailbox @a status has the EMPTY flag set. */
[[nodiscard]] static bool is_status_empty(uint32_t status) {
  return (status & STATUS_EMPTY_MASK) != 0;
}

/** Checks if given mailbox @a status has the FULL flag set. */
[[nodiscard]] static bool is_status_full(uint32_t status) {
  return (status & STATUS_FULL_MASK) != 0;
}

uint32_t receive(Channel channel) {
  uint32_t status;
  uint32_t response;

  do {
    // Make sure there is mail to receive
    do {
      status = MMIO::read(MBOX0_STATUS);
    } while (is_status_empty(status));

    // Get the message
    response = MMIO::read(MBOX0_RW);
  } while (static_cast<Channel>(response & CHANNEL_MASK) != channel);

  return response >> CHANNEL_BITS;
}

void send(Channel channel, uint32_t message) {
  uint32_t status;

  // Make sure you can send mail.
  do {
    status = MMIO::read(MBOX0_STATUS);
  } while (is_status_full(status));

  // Send the message. We write to mailbox 1 as required.
  message = static_cast<uint32_t>(channel) | (message << CHANNEL_BITS);
  MMIO::write(MBOX1_RW, message);
}
}  // namespace MailBox
