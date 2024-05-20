#pragma once

#include <cstddef>
#include <cstdint>

#include "hardware/dma/request.hpp"

namespace DMA {
class Channel {
 public:
  explicit Channel();
  ~Channel();

  /** Try to execute the list of request @a req. */
  void execute_requests(const Request& req) const;

  bool wait() const;

 private:
  const uintptr_t base;
};
}  // namespace DMA
