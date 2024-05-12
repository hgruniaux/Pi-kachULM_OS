#include "hardware/kernel_dt.hpp"

#include <libk/log.hpp>
#include "boot/mmu_utils.hpp"

static DeviceTree _dt;
static libk::StringView _board_model = "";
static uint32_t _board_revision = 0;
static uint64_t _board_serial = 0;
static Node _alias;
static Property _soc_ranges;

bool KernelDT::init(uintptr_t dtb) {
  _dt = DeviceTree(dtb);

  if (!_dt.is_status_okay()) {
    return false;
  }

  Property prop = {};

  // Fill _board_model
  if (!KernelDT::find_property("/model", &prop)) {
    return false;
  }

  _board_model = prop.get_string();

  // Fill _board_revision
  if (!KernelDT::find_property("/system/linux,revision", &prop)) {
    return false;
  }

  const auto revision = prop.get_u32();

  if (!revision.has_value()) {
    return false;
  }

  _board_revision = revision.get_value();

  // Fill _board_serial
  if (!KernelDT::find_property("/system/linux,serial", &prop)) {
    return false;
  }

  const auto serial = prop.get_u64();

  if (!serial.has_value()) {
    return false;
  }

  _board_serial = serial.get_value();

  // Fill _alias
  if (!_dt.find_node("/aliases", &_alias)) {
    return false;
  }

  // Fill _soc_ranges
  if (!KernelDT::find_property("/soc/ranges", &_soc_ranges)) {
    return false;
  }

  return true;
}

bool KernelDT::is_status_okay() {
  return _dt.is_status_okay();
}

uint32_t KernelDT::get_version() {
  return _dt.get_version();
}

ReservedSections KernelDT::get_reserved_sections() {
  ReservedSections res(nullptr);

  if (!_dt.get_reserved_sections(&res)) {
    libk::panic("[DeviceTree] Unable to retrieve reserved sections.");
  }

  return res;
}

Node KernelDT::get_root() {
  Node n = {};

  if (!_dt.get_root(&n)) {
    libk::panic("[DeviceTree] Unable to retrieve root node.");
  }

  return n;
}

bool KernelDT::find_node(libk::StringView path, Node* node) {
  return _dt.find_node(path, node);
}

bool KernelDT::find_property(libk::StringView path, Property* property) {
  return _dt.find_property(path, property);
}

libk::StringView KernelDT::get_board_model() {
  return _board_model;
}

uint32_t KernelDT::get_board_revision() {
  return _board_revision;
}

uint64_t KernelDT::get_board_serial() {
  return _board_serial;
}

Node KernelDT::get_device_node(libk::StringView device) {
  Property prop;

  if (!_alias.find_property(device, &prop)) {
    libk::panic("[DeviceTree] Unable to resolve device alias.");
  }

  Node device_node;

  if (!_dt.find_node(prop.get_string(), &device_node)) {
    libk::panic("[DeviceTree] Unable to resolve device alias.");
  }

  return device_node;
}

uintptr_t KernelDT::get_device_address(libk::StringView device) {
  Node dev_node = get_device_node(device);

  Property prop;

  if (!dev_node.find_property("reg", &prop)) {
    libk::panic("[DeviceTree] Unable to retrieve device address property.");
  }

  size_t index = 0;
  uintptr_t dev_soc_addr = 0;

  if (!prop.get_variable_int(&index, &dev_soc_addr, _mem_prop->is_soc_mem_address_u64)) {
    libk::panic("[DeviceTree] Unable to parse device address.");
  }

  index = 0;
  size_t offset = 0;
  while (index < _soc_ranges.length) {
    uint64_t soc_start = 0;
    uint64_t chunk_size = 0;

    if (!_soc_ranges.get_variable_int(&index, &soc_start, _mem_prop->is_soc_mem_address_u64)) {
      libk::panic("Unable to parse '/soc/ranges'.");
    }
    if (!_soc_ranges.get_variable_int(&index, nullptr, _mem_prop->is_arm_mem_address_u64)) {
      libk::panic("Unable to parse '/soc/ranges'.");
    }
    if (!_soc_ranges.get_variable_int(&index, &chunk_size, _mem_prop->is_soc_mem_size_u64)) {
      libk::panic("Unable to parse '/soc/ranges'.");
    }

    if (soc_start <= dev_soc_addr && dev_soc_addr < soc_start + chunk_size) {
      const size_t dev_offset = dev_soc_addr - soc_start;
      return DEVICE_MEMORY + offset + dev_offset;
    }

    offset += chunk_size;
  }

  LOG_ERROR("Unable to find corresponding virtual address of {}.", device);
  libk::panic("Unable to find virtual address.");
}
