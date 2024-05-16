/* Implements ARM-specific startup code. Notably, it invokes the C++ global
 * constructors/destructors, ensuring proper initialization of global and static
 * objects before program execution. It also clears the BSS section and invokes
 * the kernel entry point kmain().
 */

#include "hardware/device.hpp"
#include "hardware/gpio.hpp"
#include "hardware/irq/irq_manager.hpp"
#include "hardware/kernel_dt.hpp"
#include "hardware/mailbox.hpp"
#include "hardware/system_timer.hpp"

// The linker provides the following pointers.
extern uint64_t __bss_start;
extern uint64_t __bss_end;

/** Erases the BSS section. */
void zero_bss() {
  uint64_t* dst = &__bss_start;
  while (dst < &__bss_end) {
    *dst++ = 0;
  }
}

using FunctionPointer = void (*)();

// The linker provides the following pointers.
extern FunctionPointer __init_array_start[];
extern FunctionPointer __init_array_end[];
extern FunctionPointer __fini_array_start[];
extern FunctionPointer __fini_array_end[];

/** Calls all functions registered in the .init_array section.
 * This section is populated by the C and C++ compiler to implement global
 * constructors. */
void call_init_array() {
  // Call the constructors.
  auto* array = __init_array_start;
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
  auto* array = __fini_array_start;
  while (array < __fini_array_end) {
    (*array)();
    array++;
  }
}

// This function is defined in kernel.cpp. It is the real entry point of the kernel.
void kmain();

extern "C" void init_interrupts_vector_table();

/** The C and C++ world entry point. It is called from the boot.S assembly script. */
extern "C" void _startup(uintptr_t dtb) {
  // Erases the BSS section as required.
  zero_bss();

  // Set up the Interrupt Vector Table
  init_interrupts_vector_table();

  // Set up the DeviceTree
  if (!KernelDT::init(dtb)) {
    libk::halt();
  }

  // Set up the Kernel memory management
  KernelMemory::init();

  // Set up the VC-ARM Mailbox
  MailBox::init();

  // Set up general Device functions
  if (!Device::init()) {
    libk::halt();
  }

  // Set up the IRQ Manager
  IRQManager::init();

  // Set up the System Timer
  SystemTimer::init();

  // Set up GPIO Function.
  GPIO::init();

  call_init_array();
  kmain();  // the real kernel entry point
  call_fini_array();
}
