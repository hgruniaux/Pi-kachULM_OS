#pragma once

#include <cstdint>

#include <libk/assert.hpp>
#include "../boot/mmu_utils.hpp"
#include "libk/log.hpp"

namespace MailBox {
/** The different supported mailbox channels by our kernel. */
enum class Channel : uint8_t {
  /** Property channel ARM CPU to VideoCore. */
  TagArmToVC = 8,
  /** Property channel VideoCore to ARM CPU. */
  TagVCToArm = 9,
};

/** Receives a 28-bit raw message from mailbox in the requested @a channel. */
uint32_t receive(Channel channel);
/** Sends a 28-bit @a message to mailbox in the requested @a channel. */
void send(Channel channel, uint32_t message);

template <uint32_t Id, class Buffer>
struct alignas(alignof(uint32_t)) PropertyTag {
  [[maybe_unused]] uint32_t id = Id;
  [[maybe_unused]] uint32_t buffer_size = sizeof(Buffer);
  volatile uint32_t status = 0;  // Can be modified by the GPU
  volatile Buffer buffer = {};   // Can be modified by the GPU
};

template <class Tag>
struct alignas(16) PropertyMessage {
  [[maybe_unused]] uint32_t buffer_size = sizeof(PropertyMessage);
  volatile uint32_t status = 0;  // Can be modified by the GPU
  volatile Tag tag = {};         // Can be modified by the GPU
  [[maybe_unused]] uint32_t end_tag = 0;
};

[[nodiscard]] static inline bool check_tag_status(uint32_t status) {
  // bit 31 should be set to 1
  return (status >> 31) == 1;
}

template <class Message>
bool send_property(Message& message) {
  static_assert(alignof(Message) >= 16, "property messages must be 16-bytes aligned");

  const uint32_t addr = (uint32_t)((uintptr_t)&message >> 4);  // TODO: Convert Address Here
  MailBox::send(MailBox::Channel::TagArmToVC, addr);
  const uint32_t response = MailBox::receive(MailBox::Channel::TagArmToVC);
  KASSERT(response == addr);
  constexpr uint32_t STATUS_SUCCESS = 0x80000000;
  return message.status == STATUS_SUCCESS;
}
}  // namespace MailBox
