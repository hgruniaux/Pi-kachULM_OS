cmake_minimum_required(VERSION 3.20)

project(Pi-kachULM_OS LANGUAGES CXX C ASM)

# Enable C++20 (with GNU extensions)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Enable C11 (with GNU extensions)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Enable warnings.
add_compile_options(-Wall -Wextra)

# Tell the compiler that we are in a freestanding environment.
add_compile_options(-ffreestanding -nostdlib -fno-builtin)

# Disable exceptions and RTTI in C++
set(EXTRA_CXX_FLAGS -fno-exceptions -fno-rtti -mgeneral-regs-only)
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${EXTRA_CXX_FLAGS}>")

# Use relative paths in __FILE__ macros and debug information.
# This allow smaller and more readable logs.
add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}/=./)

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

add_subdirectory(lib/libk)
add_subdirectory(lib/device-tree)
add_subdirectory(kernel)
