#pragma once

#include <dtb/dtb.hpp>

namespace KernelDT {
bool init(uintptr_t dtb);

[[nodiscard]] bool is_status_okay();
[[nodiscard]] uint32_t get_version();

[[nodiscard]] ReservedSections get_reserved_sections();
[[nodiscard]] Node get_root();

[[nodiscard]] bool find_node(libk::StringView path, Node* node);
[[nodiscard]] bool find_property(libk::StringView path, Property* property);

[[nodiscard]] libk::StringView get_board_model();
[[nodiscard]] uint32_t get_board_revision();
[[nodiscard]] uint64_t get_board_serial();

[[nodiscard]] Node get_device_node(libk::StringView device);
[[nodiscard]] uintptr_t get_device_address(libk::StringView device);

};  // namespace KernelDT
