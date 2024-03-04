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

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

message("Hey: ${CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES}")
message("And: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")

# Enable checks
option(ENABLE_CHECKS "Enable checks using clang-tidy" OFF)
if (${ENABLE_CHECKS})
    find_program(CLANG_TIDY clang-tidy)
    if (CLANG_TIDY)
        set(CMAKE_CXX_CLANG_TIDY clang-tidy-15 --config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy)
    endif ()
endif ()

add_subdirectory(kernel)
