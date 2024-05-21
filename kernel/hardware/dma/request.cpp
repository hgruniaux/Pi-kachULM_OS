#include "request.hpp"
#include <climits>
#include "libk/log.hpp"
#include "memory/mem_alloc.hpp"

namespace DMA {
/** Don’t do wide writes as a 2 beat burst.
 * This prevents the DMA from issuing wide writes as 2 beat AXI bursts.
 * This is an inefficient access mode, so the default is to use the bursts. */
inline static constexpr uint32_t TI_NO_WIDE_BURSTS = 1 << 26;

/** Source address increments after each read. */
inline static constexpr uint32_t TI_SRC_INC = 1 << 8;

/** Destination address increments after each write. */
inline static constexpr uint32_t TI_DEST_INC = 1 << 4;

/** Wait for a Write Response. */
inline static constexpr uint32_t TI_WAIT_RESP = 1 << 3;

/** Enable 2D Mode. */
inline static constexpr uint32_t TI_TD_MODE = 1 << 1;

/** Interrupt Enable. */
// inline static constexpr uint32_t TI_INT_EN = 1 << 0;

struct Request::DMAStruct {
  uint32_t ti;
  uint32_t src;
  uint32_t dst;
  uint32_t length;
  uint32_t stride;
  uint32_t next_req;
  uint32_t res[2] = {0};
};

Request::Request() : dma_s((DMAStruct*)(kmalloc(sizeof(DMAStruct), 256 / CHAR_BIT))), next_req(nullptr) {}

Request::~Request() {
  kfree(dma_s);
}

Request::Request(Address src, Address dest, uint32_t length) : Request() {
  dma_s->ti = TI_SRC_INC | TI_DEST_INC | TI_WAIT_RESP | TI_NO_WIDE_BURSTS;
  dma_s->src = src;
  dma_s->dst = dest;
  dma_s->length = length;
  dma_s->stride = 0;
  dma_s->next_req = 0;
  dma_s->res[0] = 0;
  dma_s->res[1] = 0;
}

Request::Request(Address src,
                 Address dest,
                 uint16_t x_length,
                 uint16_t y_length,
                 uint16_t src_strid,
                 uint16_t dst_stride)
    : Request() {
  dma_s->ti = TI_SRC_INC | TI_DEST_INC | TI_WAIT_RESP | TI_TD_MODE | TI_NO_WIDE_BURSTS;
  dma_s->src = src;
  dma_s->dst = dest;
  dma_s->length = (y_length << 16) | x_length;
  dma_s->stride = (dst_stride << 16) | src_strid;
  dma_s->next_req = 0;
  dma_s->res[0] = 0;
  dma_s->res[1] = 0;
}

Request Request::memcpy(Address src, Address dest, uint32_t length) {
  return Request(src, dest, length);
}

Request Request::memcpy_2d(Address src,
                           Address dst,
                           uint16_t x_length,
                           uint16_t y_length,
                           uint16_t src_strid,
                           uint16_t dst_stride) {
  return Request(src, dst, x_length, y_length, src_strid, dst_stride);
}

Request* Request::link_to(Request& next) {
  auto* old_next = next_req;
  next_req = &next;
  dma_s->next_req = DMA::get_dma_bus_address((uintptr_t)next.dma_s, false);
  return old_next;
}

Request* Request::unlink() {
  if (next_req == nullptr) {
    return nullptr;
  }

  auto* old_next = next_req;
  next_req = nullptr;
  dma_s->next_req = 0;

  return old_next;
}

Request* Request::next() const {
  return next_req;
}

}  // namespace DMA
