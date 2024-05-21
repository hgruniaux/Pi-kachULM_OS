#include "dma_impl.hpp"

#include <libk/log.hpp>
#include "boot/mmu_utils.hpp"
#include "hardware/kernel_dt.hpp"
#include "hardware/timer.hpp"
#include "memory/kernel_internal_memory.hpp"

/** Interrupt status of each DMA channel. */
// inline static constexpr uint32_t INT_STATUS = 0xfe0;

/** Global enable bits for each DMA channel. */
inline static constexpr uint32_t ENABLE = 0xff0;

/** Size reserved memory for all the registers of a DMA Channel. */
static inline constexpr size_t CHANNEL_REGS_SIZE = 0x100;

/** Time to wait of the activation of a DMA Channel. */
static inline constexpr size_t ENABLE_WAIT_MS = 3;

namespace dma_impl {

static uintptr_t _dma_base = 0;
static Property _soc_dma_range;
static uint16_t _dma_channels;

bool init() {
  /* Fill _dma_base */
  if (!KernelDT::get_device_address("dma", &_dma_base)) {
    return false;
  }

  /* Fill _soc_dma_range */
  if (!KernelDT::find_property("/soc/dma-ranges", &_soc_dma_range)) {
    return false;
  }

  /* Fill _dma_channels */
  Node node;
  if (!KernelDT::get_device_node("dma", &node)) {
    return false;
  }

  Property mask;
  if (!node.find_property("brcm,dma-channel-mask", &mask)) {
    return false;
  }

  const auto dma_mask = mask.get_u32();
  if (!dma_mask.has_value()) {
    return false;
  }

  _dma_channels = dma_mask.get_value();

  return true;
}

uintptr_t allocate_channel() {
  KASSERT(_dma_base != 0);

  for (int i = 6; i >= 0; i--) {
    if (_dma_channels & (1 << i)) {
      _dma_channels &= ~(1 << i);
      return _dma_base + i * CHANNEL_REGS_SIZE;
    }
  }

  libk::panic("No DMA Channel free.");
}

void free_channel(uintptr_t chan_base) {
  const size_t chan_id = (chan_base - _dma_base) / CHANNEL_REGS_SIZE;
  _dma_channels |= 1 << chan_id;
}

void set_channel_enable(uintptr_t chan_base, bool enable) {
  const size_t chan_id = (chan_base - _dma_base) / CHANNEL_REGS_SIZE;

  const auto old_enable_value = libk::read32(_dma_base + ENABLE);
  const uint32_t mask = 1u << chan_id;
  if (enable) {
    libk::write32(_dma_base + ENABLE, old_enable_value | mask);
  } else {
    libk::write32(_dma_base + ENABLE, old_enable_value & ~mask);
  }

  /* Wait a little bit. */
  const auto target = GenericTimer::get_elapsed_time_in_ms() + ENABLE_WAIT_MS;
  while (GenericTimer::get_elapsed_time_in_ms() < target) {
    libk::yield();
  }
}

uintptr_t get_dma_bus_address(VirtualAddress va_addr, bool read_only_address) {
  PhysicalAddress pa = memory_impl::resolve_kernel_va(va_addr, read_only_address);

  size_t index = 0;
  while (index < _soc_dma_range.length) {
    uint64_t soc_start = 0;
    uint64_t arm_start = 0;
    uint64_t length = 0;

    if (!_soc_dma_range.get_variable_int(&index, &soc_start, _mem_prop->is_soc_mem_address_u64)) {
      libk::panic("Unable to parse '/soc/ranges'.");
    }
    if (!_soc_dma_range.get_variable_int(&index, &arm_start, _mem_prop->is_arm_mem_address_u64)) {
      libk::panic("Unable to parse '/soc/ranges'.");
    }
    if (!_soc_dma_range.get_variable_int(&index, &length, _mem_prop->is_soc_mem_size_u64)) {
      libk::panic("Unable to parse '/soc/ranges'.");
    }

    if (arm_start <= pa && pa < arm_start + length) {
      const size_t offset = pa - arm_start;
      return soc_start + offset;
    }
  }

  LOG_ERROR("Unable to find corresponding DMA address of {:#x}.", va_addr);
  libk::panic("Unable to find DMA address.");
}

}  // namespace dma_impl
