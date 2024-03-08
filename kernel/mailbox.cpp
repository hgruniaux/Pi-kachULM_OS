#include "mailbox.hpp"
#include "debug.hpp"
#include "mmio.hpp"

namespace MailBox {
DEBUG_IMPL_LOGGER(mailbox_logger);

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
static constexpr uint32_t MBOX1_STATUS = BASE1 + 0x18;

static constexpr uint32_t CHANNEL_WIDTH = 4;
static constexpr uint32_t CHANNEL_MASK = (1 << CHANNEL_WIDTH) - 1;

/** Checks if given mailbox @a status has the EMPTY flag set. */
[[nodiscard]] static bool is_status_empty(uint32_t status) {
  static constexpr uint32_t STATUS_EMPTY_MASK = (1 << 30);
  return (status & STATUS_EMPTY_MASK) != 0;
}

/** Checks if given mailbox @a status has the FULL flag set. */
[[nodiscard]] static bool is_status_full(uint32_t status) {
  static constexpr uint32_t STATUS_FULL_MASK = (1 << 31);
  return (status & STATUS_FULL_MASK) != 0;
}

uint32_t receive(Channel channel) {
  uint32_t status;
  uint32_t response;

  // We try to read a mail until we find one from the requested channel.
  do {
    // Wait until there is a mail to receive.
    do {
      status = MMIO::read(MBOX0_STATUS);
    } while (is_status_empty(status));

    // Get the message.
    response = MMIO::read(MBOX0_RW);
  } while (static_cast<Channel>(response & CHANNEL_MASK) != channel);

  return response >> CHANNEL_WIDTH;
}

void send(Channel channel, uint32_t message) {
  uint32_t status;

  // Wait until we can send a mail.
  do {
    status = MMIO::read(MBOX0_STATUS);
  } while (is_status_full(status));

  // Send the message. The protocol requires that we read from mailbox0
  // but that we write to mailbox1.
  message = (static_cast<uint32_t>(channel) & CHANNEL_MASK) | (message << CHANNEL_WIDTH);
  MMIO::write(MBOX1_RW, message);
}
}  // namespace MailBox
