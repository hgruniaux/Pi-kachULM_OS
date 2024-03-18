add_executable(kernel boot.S kernel.cpp cxxabi.cpp startup.cpp
        debug.hpp
        debug.cpp

        hardware/mmio.hpp
        hardware/mmio.cpp

        hardware/gpio.hpp
        hardware/gpio.cpp

        hardware/utils.hpp

        hardware/miniuart.hpp
        hardware/miniuart.cpp

        hardware/mailbox.hpp
        hardware/mailbox.cpp

        hardware/uart.hpp
        hardware/uart.cpp

        device.hpp
        device.cpp
        framebuffer.hpp
        framebuffer.cpp
        graphics/pkfont.hpp
        graphics/pkfont.cpp

        ../fonts/firacode_16.cpp
        graphics/graphics.hpp
        graphics/graphics.cpp
)

set_target_properties(kernel PROPERTIES OUTPUT_NAME "kernel8.elf")

# Use the linker script.
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/link.ld)
target_link_options(kernel PRIVATE -nostdlib -T ${LINKER_SCRIPT})
set_property(TARGET kernel PROPERTY LINK_DEPENDS ${LINKER_SCRIPT})

# Enable warnings.
target_compile_options(kernel PRIVATE -Wall -Wextra)

# Tell the compiler the CPU model.
target_compile_options(kernel PRIVATE ${CPU_OPTIONS})

# Tell the compiler that we are in a freestanding environment.
target_compile_options(kernel PRIVATE -ffreestanding -nostdlib)

# Disable exceptions and RTTI in C++
target_compile_options(kernel PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti -fno-builtin>)

# Use relative paths in __FILE__ macros and debug information.
# This allow smaller and more readable logs.
target_compile_options(kernel PRIVATE -ffile-prefix-map=${CMAKE_SOURCE_DIR}/=./)

# Extracting the elf file to a binary one
add_custom_target(kernel-img
        COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/kernel8.elf ${CMAKE_CURRENT_BINARY_DIR}/kernel8.img
        DEPENDS kernel
        COMMENT "Converting ELF to binary")
