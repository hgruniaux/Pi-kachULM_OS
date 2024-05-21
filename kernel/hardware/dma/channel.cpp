#include "channel.hpp"
#include <libk/utils.hpp>

#include "dma_impl.hpp"
#include "memory/kernel_internal_memory.hpp"

namespace DMA {

/** Activate the DMA */
inline static constexpr uint32_t CS_ACTIVE = 1 << 0;

/** DMA End Flag */
inline static constexpr uint32_t CS_END = 1 << 1;

/** Interrupt Status */
inline static constexpr uint32_t CS_INT = 1 << 2;

/** DMA Error. Indicates if the DMA has detected an error. */
inline static constexpr uint32_t CS_ERROR = 1 << 8;

/** Wait for outstanding writes */
inline static constexpr uint32_t CS_WAIT_FOR_WRITE = 1 << 28;

/** Disable debug pause signal */
inline static constexpr uint32_t CS_DIS_DEBUG = 1 << 29;

/** Abort Current DMA Request and start the next. */
inline static constexpr uint32_t CS_ABORT = 1 << 30;

/** DMA Channel Reset */
inline static constexpr uint32_t CS_RESET = 1 << 31;

/** DMA Control and Status register contains the main control and status bits for this DMA channel. */
inline static constexpr uint32_t CS = 0;

/** DMA Control Block Address register. */
inline static constexpr uint32_t CONBLK_AD = 0x4;

/** DMA Debug register. */
// inline static constexpr uint32_t DEBUG = 0x20;

Channel::Channel() : base(dma_impl::allocate_channel()) {
  dma_impl::set_channel_enable(base, true);

  /* Resetting the channel */
  const auto ctrl_value = libk::read32(base + CS);
  libk::write32(base + CS, ctrl_value | CS_RESET);
  while ((libk::read32(base + CS) & CS_RESET) != 0) {
    libk::yield();
  }
}

Channel::~Channel() {
  dma_impl::set_channel_enable(base, false);
  dma_impl::free_channel(base);
}

bool Channel::execute_requests(const Request& req) const {
  if (!is_free()) {
    return false;
  }

  const uintptr_t req_va_address = (uintptr_t)req.dma_s;
  const uintptr_t req_address = memory_impl::resolve_kernel_va(req_va_address, false);

  libk::write32(base + CONBLK_AD, req_address);
  libk::write32(base + CS, CS_WAIT_FOR_WRITE | CS_DIS_DEBUG | CS_ACTIVE | 15 << 20 | 8 << 16);
  return true;
}

bool Channel::wait() const {
  while (!is_free()) {
    libk::yield();
  }

  return has_error();
}
bool Channel::is_free() const {
  return ((libk::read32(base + CS) & CS_ACTIVE) == 0);
}

bool Channel::has_error() const {
  return (libk::read32(base + CS) & CS_ERROR) != 0;
}

void Channel::abort_previous() const {
  const auto old_cs = libk::read32(base + CS);

  libk::write32(base + CS, old_cs & ~(CS_ACTIVE));
  libk::write32(base + CONBLK_AD, 0);
  libk::write32(base + CS, CS_ABORT);
  libk::write32(base + CS, CS_RESET);
  libk::write32(base + CS, CS_INT | CS_END);
}

}  // namespace DMA
