#pragma once
namespace libk {
#ifdef TARGET_QEMU

[[noreturn]] static void qemu_exit(uint64_t code) {
  struct QEMUParameterBlock {
    uint64_t arg0;
    uint64_t arg1;
  };  // struct QEMUParameterBlock

  constexpr uint64_t ADP_Stopped_ApplicationExit = 0x20026;
  QEMUParameterBlock parameters = {ADP_Stopped_ApplicationExit, code};

  asm volatile(
      "mov w0, #0x18\n\t"
      "mov x1, %0\n\t"
      "hlt #0xF000"
      :
      : "r"(&parameters)
      : "x0", "x1");

  // In case of failure, just halt the system.
  while (true) {
    asm volatile("");
  }
}
#endif  // TARGET_QEMU

}  // namespace libk
