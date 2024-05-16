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
[[nodiscard]] StringList get_board_compatible();

[[nodiscard]] bool get_device_node(libk::StringView device, Node* device_node);
[[nodiscard]] bool get_device_address(libk::StringView device, uintptr_t* device_address);
[[nodiscard]] uintptr_t convert_soc_address(uintptr_t soc_address);

[[nodiscard]] uintptr_t force_get_device_address(libk::StringView device);
};  // namespace KernelDT
