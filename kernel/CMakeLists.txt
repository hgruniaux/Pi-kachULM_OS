add_executable(kernel boot.S kernel.cpp cxxabi.cpp startup.cpp
        debug.hpp
        debug.cpp

        syscall.hpp
        syscall.cpp

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

        hardware/device.hpp
        hardware/device.cpp

        hardware/framebuffer.hpp
        hardware/framebuffer.cpp

        hardware/interrupts.hpp
        hardware/interrupts.cpp
        hardware/interrupts.S

        graphics/pkfont.hpp
        graphics/pkfont.cpp

        ../fonts/firacode_16.cpp
        graphics/graphics.hpp
        graphics/graphics.cpp
)

set_target_properties(kernel PROPERTIES OUTPUT_NAME "kernel8.elf")

target_link_libraries(kernel PRIVATE libk device-tree)

# Use the linker script.
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/link.ld)
target_link_options(kernel PRIVATE -nostdlib -T ${LINKER_SCRIPT})
set_property(TARGET kernel PROPERTY LINK_DEPENDS ${LINKER_SCRIPT})

# Extracting the elf file to a binary one
add_custom_target(kernel-img
        COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/kernel8.elf ${CMAKE_CURRENT_BINARY_DIR}/kernel8.img
        DEPENDS kernel
        COMMENT "Converting ELF to binary")
