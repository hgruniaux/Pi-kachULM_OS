add_executable(kernel
        # Boot
        boot/boot.S

        # MMU Setup
        boot/mmu_utils.hpp
        boot/mmu_init.cpp

        boot/mmu_table.cpp
        boot/mmu_table.hpp

        boot/memory.cpp
        boot/memory.hpp

        # C++ Setup
        boot/startup.cpp
        boot/cxxabi.cpp
        kernel.cpp

        syscall.hpp
        syscall.cpp

        # Hardware
        hardware/interrupts.hpp
        hardware/interrupts.cpp
        hardware/interrupts.S

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

        hardware/timer.hpp

        # Graphics
        graphics/pkfont.hpp
        graphics/pkfont.cpp

        boot/page_alloc.cpp
        boot/page_alloc.hpp

        ../fonts/firacode_16.cpp
        graphics/graphics.hpp
        graphics/graphics.cpp
        kernel.hpp
        boot/mem_alloc.cpp
        boot/mem_alloc.hpp

        boot/kernel_dt.cpp
        boot/kernel_dt.hpp
        boot/mmu_utils.cpp
)

set_target_properties(kernel PROPERTIES OUTPUT_NAME "kernel8.elf")

target_link_libraries(kernel PRIVATE libk device-tree)

# Use the linker script.
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boot/link.ld)
add_custom_target(linker_script
        DEPENDS ${LINKER_SCRIPT}
        COMMAND ${CMAKE_C_COMPILER} -D__ASSEMBLER__  -E -P -x c ${LINKER_SCRIPT} -o ${CMAKE_CURRENT_BINARY_DIR}/link-gen.ld
)
target_link_options(kernel PRIVATE -nostdlib -T ${CMAKE_CURRENT_BINARY_DIR}/link-gen.ld)
# In order to be compatible with MAKE
# See: https://discourse.cmake.org/t/link-depends-on-output-of-add-custom-command-no-rule-to-make-target/10179
add_dependencies(kernel linker_script)

# Extracting the elf file to a binary one
add_custom_target(kernel-img
        COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/kernel8.elf ${CMAKE_CURRENT_BINARY_DIR}/kernel8.img
        DEPENDS kernel
        COMMENT "Converting ELF to binary")
