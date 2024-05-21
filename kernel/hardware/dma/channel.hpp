#pragma once

#include <cstddef>
#include <cstdint>

#include "hardware/dma/request.hpp"

namespace DMA {
class Channel {
 public:
  explicit Channel();
  ~Channel();

  /** Try to execute the list of request @a req.
   * @a Returns `true` is the request has been started. */
  bool execute_requests(const Request* req) const;

  /** Wait for requests to end. */
  bool wait() const;

  /** Checks if this DMA Channel is free to execute requests. */
  bool is_free() const;

  /** Checks if this DMA Channel have an error. */
  bool has_error() const;

  /** Abort all previous requests ! */
  void abort_previous() const;

 private:
  const uintptr_t base;
};
}  // namespace DMA
