#pragma once

#include <cstdint>

#include "debug.hpp"

namespace MailBox {
DEBUG_DECL_LOGGER(mailbox_logger);

/** The different supported mailbox channels by our kernel. */
enum class Channel : uint8_t {
  /** Property channel ARM CPU to VideoCore. */
  TagArmToVC = 8,
  /** Property channel VideoCore to ARM CPU. */
  TagVCToArm = 9,
};  // enum class Channel

/** Receives a 28-bit raw message from mailbox in the requested @a channel. */
uint32_t receive(Channel channel);
/** Sends a 28-bit @a message to mailbox in the requested @a channel. */
void send(Channel channel, uint32_t message);

template <uint32_t Id, class Buffer>
struct alignas(alignof(uint32_t)) PropertyTag {
  uint32_t id = Id;
  uint32_t buffer_size = sizeof(Buffer);
  uint32_t status = 0;
  Buffer buffer = {};
};  // struct PropertyTag

template <class Tag>
struct alignas(16) PropertyMessage {
  uint32_t buffer_size = sizeof(PropertyMessage);
  uint32_t status = 0;
  Tag tag = {};
  uint32_t end_tag = 0;
};  // struct PropertyMessage

template <class Message>
bool send_property(Message& message) {
  static_assert(alignof(Message) >= 16, "property messages must be 16-bytes aligned");

  const uint32_t addr = (uint32_t)((uintptr_t)&message >> 4);
  MailBox::send(MailBox::Channel::TagArmToVC, addr);
  const uint32_t response = MailBox::receive(MailBox::Channel::TagArmToVC);
  KASSERT(response == addr);
  constexpr uint32_t STATUS_SUCCESS = 0x80000000;
  return message.status == STATUS_SUCCESS;
}
}  // namespace MailBox
