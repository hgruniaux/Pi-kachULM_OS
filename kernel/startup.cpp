/* Implements ARM-specific startup code. Notably, it invokes the C++ global
 * constructors/destructors, ensuring proper initialization of global and static
 * objects before program execution. It also clears the BSS section and invokes
 * the kernel entry point kmain().
 */

#include <cstdint>

// The following pointers are provided by the linker.
extern uint32_t __bss_start;
extern uint32_t __bss_end;

/** Erases the BSS section. */
void zero_bss() {
  uint32_t* dst = &__bss_start;
  while (dst < &__bss_end) {
    *dst++ = 0;
  }
}

using FunctionPointer = void (*)();

// The following pointers are provided by the linker.
extern FunctionPointer __init_array_start[];
extern FunctionPointer __init_array_end[];
extern FunctionPointer __fini_array_start[];
extern FunctionPointer __fini_array_end[];

/** Calls all functions registered in the .init_array section.
 * This section is populated by the C and C++ compiler to implement global
 * constructors. */
void call_init_array() {
  // Call the constructors.
  auto array = __init_array_start;
  while (array < __init_array_end) {
    (*array)();
    array++;
  }
}

/** Calls all functions registered in the .fini_array section.
 * This section is populated by the C and C++ compiler to implement global
 * destructors. */
void call_fini_array() {
  // Call the destructors.
  auto array = __fini_array_start;
  while (array < __fini_array_end) {
    (*array)();
    array++;
  }
}

// This function is defined in kernel.cpp. It is the real entry point of the kernel.
extern "C" void kmain();

/** The C and C++ world entry point. It is called from the boot.S assembly script. */
extern "C" void _startup() {
  // Erases the BSS section as required.
  zero_bss();

  call_init_array();
  kmain();  // the real kernel entry point
  call_fini_array();
}
