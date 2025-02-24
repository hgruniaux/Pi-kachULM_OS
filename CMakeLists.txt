cmake_minimum_required(VERSION 3.20)

project(Pi-kachULM_OS LANGUAGES CXX C ASM)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Enable C++23 (with GNU extensions)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Enable C11 (with GNU extensions)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Enable warnings.
add_compile_options(-Wall -Wextra)

# Tell the compiler that we are in a freestanding environment.
add_compile_options(-ffreestanding -nostdlib -fno-builtin -mno-outline-atomics)

# Disable exceptions and RTTI in C++
set(EXTRA_CXX_FLAGS -fno-exceptions -fno-rtti)
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_FLAGS}>")

# Use relative paths in __FILE__ macros and debug information.
# This allow smaller and more readable logs.
add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}/=./)

# =========================================================
# Some configurations that are supported by the kernel code:

# Use the DMA hardware/driver to blit windows into the screen.
# add_compile_definitions(-DCONFIG_USE_DMA)

# Allocate a single large framebuffer per window (and do not reallocate and resize it when
# the window is resized). This is intended to fix a bug in the DMA/page allocator.
# If the DMA is disabled, this config is ignored.
add_compile_definitions(-DCONFIG_WINDOW_LARGE_FRAMEBUFFER)

# Support cursor in the window manager.
add_compile_definitions(-DCONFIG_HAS_CURSOR)

# Use a naive malloc/free implementation that just allocate memory using the heap break
# and never free memory. This is a really bad allocator (as it never free memory), but
# it is guaranteed to work.
# add_compile_definitions(-DCONFIG_USE_NAIVE_MALLOC)

# Enable the use of double buffering for the screen framebuffer (using mailbox set virtual offset).
# add_compile_definitions(-DCONFIG_USE_DOUBLE_BUFFERING)

# Enable checks
option(ENABLE_CHECKS "Enable checks using clang-tidy" OFF)
if (${ENABLE_CHECKS})
    function(check_clang_tidy_version validator_result_var item)
        # We require at least clang-tidy version 15.
        execute_process(COMMAND ${item} --version OUTPUT_VARIABLE _clang_tidy_version_output)
        string(REGEX MATCH "[\.0-9]+" _clang_tidy_version_string "${_clang_tidy_version_output}")
        if (_clang_tidy_version_string VERSION_LESS "15.0.0")
            message(STATUS "Found an invalid version of clang-tidy: ${item}")
            set(${validator_result_var} FALSE PARENT_SCOPE)
        endif ()
    endfunction()

    find_program(CLANG_TIDY NAMES clang-tidy clang-tidy-15 clang-tidy-16 clang-tidy-17 clang-tidy-18 clang-tidy-19 VALIDATOR check_clang_tidy_version)
    if (CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY} --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy)
    else ()
        message(FATAL_ERROR "ENABLE_CHECKS is ON but no valid version of clang-tidy was found")
    endif ()
endif ()

option(BUILD_TEST "Build the tests" OFF)
if (${BUILD_TEST})
    add_compile_definitions(-DBUILD_TESTS)
endif ()

option(TARGET_QEMU "Target is QEMU" OFF)
if (${TARGET_QEMU})
    add_compile_definitions(-DTARGET_QEMU)
endif ()

add_subdirectory(lib)
add_subdirectory(binuser)
add_subdirectory(kernel)

add_custom_target(Pi-kachULM_OS-img
        DEPENDS kernel-img
        COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/tools/create-boot-img.sh" "${CMAKE_CURRENT_BINARY_DIR}" "Pi-kachULM_OS.img")
