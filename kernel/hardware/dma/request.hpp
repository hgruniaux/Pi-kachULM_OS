#pragma once

#include "dma_controller.hpp"

namespace DMA {
class Channel;

class Request {
 public:
  ~Request();

  /** Create a Request that will write @a byte_length bytes from @a src to @a dst.
   * Beware, @a src is a DMA Address, so if byte_length > page_size, it must
   * be made of continuous **physical** address ! Use Buffer to have an buffer continuous in memory. */
  static Request memcpy(Address src, Address dst, uint32_t byte_length);

  /** Create a Request that will 2D write @a value on @a dst.
   *
   * @param value the 4-byte value that will be written in @a dst.
   * @param dst the target address.
   * @param line_byte_length the **byte** length of a line.
   * @param nb_line the number of line to write.
   * @param src_stride, dst_stride This value is added to pass from a line to the next one.
   *            So it is the difference from the end of a line to the start of the next one !
   *            THIS IS NOT THE PITCH.
   */
  static Request memcpy_2d(Address src,
                           Address dst,
                           uint16_t line_byte_length,
                           uint16_t nb_lines,
                           uint16_t src_stride,
                           uint16_t dst_stride);

  /** Execute the request @a next after this one.
   * The previous following request is returned and overwritten.
   * This operation modify this request and so any list in witch this request appear. */
  Request* link_to(Request& next);

  /** Remove and returns the request that shall be execute after this one. */
  Request* unlink();

  /** Get the following request. */
  Request* next() const;

 private:
  Request();

  Request(Address src, Address dest, uint32_t length);
  Request(Address src, Address dest, uint16_t x_length, uint16_t y_length, uint16_t src_strid, uint16_t dst_stride);

  friend Channel;

  struct DMAStruct;
  DMAStruct* const dma_s;
  Request* next_req;
};
}  // namespace DMA
