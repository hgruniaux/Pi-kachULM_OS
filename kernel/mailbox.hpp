#pragma once

#include <cstdint>

#include "debug.hpp"

namespace MailBox {
DEBUG_DECL_LOGGER(mailbox_logger);

/** The different supported mailbox channels by our kernel. */
enum class Channel : uint32_t {
  /** Property channel ARM CPU to VideoCore. */
  TagArmToVC = 8,
  /** Property channel VideoCore to ARM CPU. */
  TagVCToArm = 9,
};  // enum class Channel

/** Receives a 28-bit raw message from mailbox in the requested @a channel. */
uint32_t receive(Channel channel);
/** Sends a 28-bit @a message to mailbox in the requested @a channel. */
void send(Channel channel, uint32_t message);
}  // namespace MailBox
