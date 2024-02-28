/*
 * crti.cxx for ARM - BPABI
 *
 * Implements ARM-specific code to invoke C++ global constructors on the ARM
 * platform, ensuring proper initialization of global and static objects before
 * program execution.
 */

using FunctionPointer = void (*)();

extern FunctionPointer _init_array_start[0], _init_array_end[0];
extern FunctionPointer _fini_array_start[0], _fini_array_end[0];

// These two functions are called in the boot.S file. The first is called just
// before entering the kernel C code (calling the kmain() function), and the
// second just after. They simply call all registered global constructors and
// destructors.
//
// Global constructions are not only specific to C++. The GCC
// __attribute__((constructor)) also register global constructor functions.

// See https://wiki.osdev.org/Calling_Global_Constructors#ARM_.28BPABI.29

extern "C" void
_init()
{
  // Call the constructors.
  for (auto* f = _init_array_start; f != _init_array_end; f++)
    (*f)();
}

extern "C" void
_fini()
{
  // Call the destructors.
  for (auto* f = _fini_array_start; f != _fini_array_end; f++)
    (*f)();
}

// These two addresses will be set by the linker and the compiler.
FunctionPointer _init_array_start[0]
  __attribute__((used,
                 section(".init_array"),
                 aligned(sizeof(FunctionPointer)))) = {};
FunctionPointer _fini_array_start[0]
  __attribute__((used,
                 section(".fini_array"),
                 aligned(sizeof(FunctionPointer)))) = {};
