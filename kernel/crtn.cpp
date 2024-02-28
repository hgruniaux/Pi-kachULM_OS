/*
 * crtn.cxx for ARM - BPABI
 *
 * Implements ARM-specific code to invoke C++ global constructors on the ARM
 * platform, ensuring proper initialization of global and static objects before
 * program execution.
 *
 * See crti.cxx for more details.
 */

using FunctionPointer = void (*)();

// These two addresses will be set by the linker and the compiler.
FunctionPointer _init_array_end[0]
  __attribute__((used,
                 section(".init_array"),
                 aligned(sizeof(FunctionPointer)))) = {};
FunctionPointer _fini_array_end[0]
  __attribute__((used,
                 section(".fini_array"),
                 aligned(sizeof(FunctionPointer)))) = {};
