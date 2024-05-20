#include "dma_controller.hpp"

#include "hardware/dma/dma_impl.hpp"

namespace DMA {
bool init() {
  return dma_impl::init();
}

Address get_dma_bus_address(VirtualAddress va_addr, bool read_only_address) {
  return dma_impl::get_dma_bus_address(va_addr, read_only_address);
}

}  // namespace DMA
