cmake_minimum_required(VERSION 3.16)

include(cmake/GCCToolchain.cmake)

project(os LANGUAGES C CXX ASM)

# Enable C++20 (with GNU extensions)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Enable C11 (with GNU extensions)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Enable checks
option(ENABLE_CHECKS "Enable checks using clang-tidy" OFF)
if (${ENABLE_CHECKS})
  find_program(CLANG_TIDY clang-tidy)
  if (CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY clang-tidy --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy)
  endif ()
endif ()

file(GLOB_RECURSE KERNEL_SRC_FILES "kernel/*.S" "kernel/*.c" "kernel/*.cpp" "kernel/*.h" "kernel/*.hpp")
add_executable(kernel8.elf ${KERNEL_SRC_FILES})

# Use the linker script.
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/link.ld)
target_link_options(kernel8.elf PRIVATE -nostdlib -T ${LINKER_SCRIPT})
set_property(TARGET kernel8.elf PROPERTY LINK_DEPENDS ${LINKER_SCRIPT})

# Enable warnings.
target_compile_options(kernel8.elf
        PRIVATE -Wall -Wextra)

# Tell the compiler the CPU model.
target_compile_options(kernel8.elf
        PRIVATE ${CPU_OPTIONS})

# Tell the compiler that we are in a freestanding environment.
target_compile_options(kernel8.elf
        PRIVATE -ffreestanding -nostdlib -nostartfiles)

# Disable exceptions and RTTI in C++
target_compile_options(kernel8.elf
        PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti>)
