#include "channel.hpp"
#include <libk/utils.hpp>

#include "dma_impl.hpp"
#include "memory/kernel_internal_memory.hpp"

namespace DMA {

/** Activate the DMA */
inline static constexpr uint32_t CS_ACTIVE = 1 << 0;

/** DMA Error. Indicates if the DMA has detected an error. */
inline static constexpr uint32_t CS_ERROR = 1 << 8;

/** Wait for outstanding writes */
inline static constexpr uint32_t CS_WAIT_FOR_WRITE = 1 << 28;

/** Disable debug pause signal */
inline static constexpr uint32_t CS_DIS_DEBUG = 1 << 29;

/** DMA Channel Reset */
inline static constexpr uint32_t CS_RESET = 1 << 31;

/** DMA Control and Status register contains the main control and status bits for this DMA channel. */
inline static constexpr uint32_t CS = 0;

/** DMA Control Block Address register. */
inline static constexpr uint32_t CONBLK_AD = 0x4;

/** DMA Debug register. */
//inline static constexpr uint32_t DEBUG = 0x20;

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

void Channel::execute_requests(const Request& req) const {
  const uintptr_t req_va_address = (uintptr_t)req.dma_s;
  const uintptr_t req_address = memory_impl::resolve_kernel_va(req_va_address, false);

  libk::write32(base + CONBLK_AD, req_address);
  libk::write32(base + CS, CS_WAIT_FOR_WRITE | CS_DIS_DEBUG | CS_ACTIVE | 15 << 20 | 8 << 16);
}

bool Channel::wait() const {
  while ((libk::read32(base + CS) & CS_ACTIVE) != 0) {
    //    LOG_DEBUG("[CS] We have {:#x}", libk::read32(base + CS));
    libk::yield();
  }

  return (libk::read32(base + CS) & CS_ERROR) != 0;
}

}  // namespace DMA
