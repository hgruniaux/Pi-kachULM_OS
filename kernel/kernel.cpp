#include <cstddef>
#include <cstdint>

#include "mmio.hpp"
#include "uart.hpp"
#include "mailbox.hpp"

extern "C" void kmain() {
  MMIO::init();
  UART::init();
  UART::puts("Hello, kernel World!\r\n");

  struct GetBoardModelTag {
    uint32_t tag = 0x00010001;
    uint32_t tag_size = sizeof(GetBoardModelTag);
    uint32_t board_model = 0;
  }; // struct GetBoardModelTag

  struct GetBoardRevisionTag {
    uint32_t tag = 0x00010002;
    uint32_t tag_size = sizeof(GetBoardRevisionTag);
    uint32_t board_revision = 0;
  }; // struct GetBoardRevisionTag

  struct GetBoardMACAddressTag {
    uint32_t tag = 0x00010003;
    uint32_t tag_size = sizeof(GetBoardMACAddressTag);
    uint8_t mac_address[8] = { 0 };
  }; // struct GetBoardRevisionTag

  struct alignas(16) Request {
    uint32_t buffer_size = 0;
    uint32_t request = 0;
    GetBoardModelTag board_model_tag;
    GetBoardRevisionTag board_revision_tag;
    GetBoardMACAddressTag board_mac_address_tag;
    uint32_t end_tag = 0;
  };

  Request request;
  request.buffer_size = sizeof(Request);
  MailBox::send(MailBox::Channel::TagArmToVC, (uint32_t)((uintptr_t)&request) >> 4);
  MailBox::receive(MailBox::Channel::TagArmToVC);

  LOG_INFO("Board model: {}", request.board_model_tag.board_model);
  LOG_INFO("Board revision: {}", request.board_revision_tag.board_revision);

  while (true)
    UART::write_one(UART::read_one());
}
