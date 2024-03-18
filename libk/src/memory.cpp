#include <cstdint>
#include <cstddef>

namespace libk::detail {
void* memset(void* dest, uint8_t value, size_t count) {

}
}  // namespace libk::detail

#if LIBK_ENABLE_C
extern "C" void* memset(void* dest, int value, size_t count) {
  return libk::detail::memset(dest, static_cast<uint8_t>(value), count);
}
#endif  // LIBK_ENABLE_C
