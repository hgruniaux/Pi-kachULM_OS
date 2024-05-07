#include "kernel_dt.hpp"
#include <libk/log.hpp>
#include "mmu_utils.hpp"

DeviceTree _dt;
libk::StringView _board_model = "";
uint32_t _board_revision = 0;
uint64_t _board_serial = 0;
Node _alias;

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
    LOG_CRITICAL("[DeviceTree] Unable to retrieve reserved sections.");
  }

  return res;
}

Node KernelDT::get_root() {
  Node n = {};

  if (!_dt.get_root(&n)) {
    LOG_CRITICAL("[DeviceTree] Unable to retrieve root node.");
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
    LOG_CRITICAL("[DeviceTree] Unable to resolve device alias.");
  }

  Node device_node;

  if (!_dt.find_node(prop.get_string(), &device_node)) {
    LOG_CRITICAL("[DeviceTree] Unable to resolve device alias.");
  }

  return device_node;
}

uintptr_t KernelDT::get_device_mmio_address(libk::StringView device) {
  Node dev_node = get_device_node(device);

  Property prop;

  if (!dev_node.find_property("reg", &prop)) {
    LOG_CRITICAL("[DeviceTree] Unable to retrieve device address property.");
  }

  size_t index = 0;
  uintptr_t address = 0;

  if (!prop.get_variable_int(&index, &address, _mem_prop->is_soc_mem_address_u64)) {
    LOG_CRITICAL("[DeviceTree] Unable to parse device address.");
  }

  return address;
}
