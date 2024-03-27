add_executable(kernel boot.S
        kernel.cpp
        debug.hpp debug.cpp
        cxxabi.cpp
        startup.cpp
        mmio.hpp mmio.cpp
        uart.hpp uart.cpp
        mailbox.hpp mailbox.cpp
)

set_target_properties(kernel PROPERTIES OUTPUT_NAME "kernel8.elf")

target_link_libraries(kernel PRIVATE libk)

# Use the linker script.
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/link.ld)
target_link_options(kernel PRIVATE -nostdlib -T ${LINKER_SCRIPT})
set_property(TARGET kernel PROPERTY LINK_DEPENDS ${LINKER_SCRIPT})

# Extracting the elf file to a binary one
add_custom_target(kernel-img
        COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/kernel8.elf ${CMAKE_CURRENT_BINARY_DIR}/kernel8.bin
        DEPENDS kernel
        COMMENT "Converting ELF to binary")
