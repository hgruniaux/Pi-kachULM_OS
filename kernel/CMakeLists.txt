add_executable(kernel
        # Boot
        boot/boot.S

        boot/mmu_init.cpp
        boot/startup.cpp
        boot/cxxabi.cpp

        boot/mmu_utils.hpp
        boot/mmu_utils.cpp

        # C++ Body
        kernel.cpp

        # Memory
        memory/mmu_table.hpp
        memory/mmu_table.cpp

        memory/kernel_internal_memory.hpp
        memory/kernel_internal_memory.cpp

        memory/memory.hpp
        memory/memory.cpp

        memory/page_alloc.hpp
        memory/page_alloc.cpp

        memory/mem_alloc.hpp
        memory/mem_alloc.cpp

        memory/page_alloc_list.hpp
        memory/page_alloc_list.cpp

        memory/heap_manager.hpp
        memory/heap_manager.cpp

        memory/memory_chunk.hpp
        memory/memory_chunk.cpp

        memory/process_memory.hpp
        memory/process_memory.cpp

        memory/buffer.hpp
        memory/buffer.cpp

        memory/contiguous_page_alloc.hpp
        memory/contiguous_page_alloc.cpp

        # Hardware
        hardware/interrupts.hpp
        hardware/interrupts.cpp
        hardware/interrupts.S

        hardware/kernel_dt.hpp
        hardware/kernel_dt.cpp

        hardware/gpio.hpp
        hardware/gpio.cpp

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

        hardware/system_timer.hpp
        hardware/system_timer.cpp

        hardware/regs.hpp

        # IRQ
        hardware/irq/bcm2837_irq_manager.hpp
        hardware/irq/bcm2837_irq_manager.cpp

        hardware/irq/bcm2711_irq_manager.hpp
        hardware/irq/bcm2711_irq_manager.cpp

        hardware/irq/irq_lists.hpp

        hardware/irq/irq_manager.hpp
        hardware/irq/irq_manager.cpp

        # DMA
        hardware/dma/dma_impl.hpp
        hardware/dma/dma_impl.cpp

        hardware/dma/channel.hpp
        hardware/dma/channel.cpp

        hardware/dma/request.hpp
        hardware/dma/request.cpp

        hardware/dma/dma_controller.hpp
        hardware/dma/dma_controller.cpp

        # Graphics
        graphics/pkfont.hpp
        graphics/pkfont.cpp

        ../fonts/firacode_16.cpp

        graphics/graphics.hpp
        graphics/graphics.cpp

        graphics/stb_image.h
        graphics/stb_image.c

        # Task
        task/task_manager.hpp
        task/task_manager.cpp

        task/scheduler.hpp
        task/scheduler.cpp

        task/syscall_table.hpp
        task/syscall_table.cpp

        task/task.hpp
        task/task.cpp

        task/pika_syscalls.hpp
        task/pika_syscalls.cpp

        task/wait_list.hpp
        task/wait_list.cpp

        task/delta_queue.hpp
        task/delta_queue.cpp

        # Window manager
        wm/geometry.hpp

        wm/window_manager.cpp
        wm/window_manager.hpp

        wm/window.cpp
        wm/window.hpp

        wm/message_queue.cpp
        wm/message_queue.hpp

        # Window manager data: icons and wallpaper
        wm/data/pika_icon.hpp
        wm/data/pika_icon.cpp
        wm/data/wallpaper.hpp
        wm/data/wallpaper.cpp

        # Filesystem
        fs/filesystem.hpp
        fs/filesystem.cpp
        fs/fat/ff.c
        fs/fat/ff.h
        fs/fat/ffconf.h
        fs/fat/ffsystem.cpp
        fs/fat/ffunicode.c
        fs/fat/ramdisk.cpp
        fs/fat/ramdisk.hpp
)

target_include_directories(kernel PUBLIC .)
set_target_properties(kernel PROPERTIES OUTPUT_NAME "kernel8.elf")

target_link_libraries(kernel PRIVATE libk libsyscall libelf device-tree)

add_executable(init init.c)
target_link_libraries(init PRIVATE libsyscall)
target_compile_options(init PRIVATE -nostdlib -no-pie -Wl,-e_start)
target_link_options(init PRIVATE -nostdlib -no-pie -Wl,-e_start)
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/init_elf.c
        COMMAND xxd -i init > ${CMAKE_CURRENT_BINARY_DIR}/init_elf.c
        DEPENDS init
        COMMENT "Embedding ELF init in kernel")

# Use the linker script.
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boot/link.ld)
add_custom_target(linker_script
        DEPENDS ${LINKER_SCRIPT}
        COMMAND ${CMAKE_C_COMPILER} -D__ASSEMBLER__ -E -P -x c ${LINKER_SCRIPT} -o ${CMAKE_CURRENT_BINARY_DIR}/link-gen.ld
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
