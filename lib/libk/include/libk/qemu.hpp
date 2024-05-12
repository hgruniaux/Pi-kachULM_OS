#pragma once

#include <cstdint>

namespace libk {
#ifdef TARGET_QEMU
[[noreturn]] void qemu_exit(uint64_t code);
#endif  // TARGET_QEMU
}  // namespace libk
