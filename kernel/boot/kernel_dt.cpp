#include "kernel_dt.hpp"

DeviceTree dt;
libk::StringView _board_model = "";
uint32_t _board_revision = 0;
uint64_t _board_serial = 0;

bool KernelDT::init(uintptr_t dtb) {
  dt = DeviceTree(dtb);

  if (!dt.is_status_okay()) {
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

  return true;
}

bool KernelDT::is_status_okay() {
  return dt.is_status_okay();
}

uint32_t KernelDT::get_version() {
  return dt.get_version();
}

ReservedSections KernelDT::get_reserved_sections() {
  ReservedSections res(nullptr);

  if (!dt.get_reserved_sections(&res)) {
    libk::halt();
  }

  return res;
}

Node KernelDT::get_root() {
  Node n = {};

  if (!dt.get_root(&n)) {
    libk::halt();
  }

  return n;
}

bool KernelDT::find_node(libk::StringView path, Node* node) {
  return dt.find_node(path, node);
}

bool KernelDT::find_property(libk::StringView path, Property* property) {
  return dt.find_property(path, property);
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

uintptr_t KernelDT::get_device_mmio_address(libk::StringView device) {
  return 0;
}
