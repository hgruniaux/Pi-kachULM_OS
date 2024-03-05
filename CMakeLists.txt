cmake_minimum_required(VERSION 3.16)

include(cmake/GCCToolchain.cmake)

project(Pi-kachULM_OS LANGUAGES CXX ASM)

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
    function(check_clang_tidy_version validator_result_var item)
        # We require at least clang-tidy version 15.
        execute_process(COMMAND ${item} --version OUTPUT_VARIABLE _clang_tidy_version_output)
        string(REGEX REPLACE "version (\\d+).(\\d+).(\\d+)" "\\1" _clang_tidy_version_major ${_clang_tidy_version_output})
        if (_clang_tidy_version_major LESS 15)
            message(STATUS "Found an invalid version of clang-tidy: ${item}")
            set(${validator_result_var} FALSE PARENT_SCOPE)
        endif ()
    endfunction()

    find_program(CLANG_TIDY NAMES clang-tidy clang-tidy-15 clang-tidy-16 clang-tidy-17 clang-tidy-18 clang-tidy-19 VALIDATOR check_clang_tidy_version)
    if (CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY} --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy -extra-arg=-I${CMAKE_SYSROOT}/include -extra-arg=-I${CMAKE_SYSROOT}/include/c++/13.2.1 -extra-arg=-I${CMAKE_SYSROOT}/include/c++/13.2.1/aarch64-none-elf)
    else ()
        message(WARNING "ENABLE_CHECKS is ON but no valid version of clang-tidy was found")
    endif ()
endif ()

add_subdirectory(kernel)
