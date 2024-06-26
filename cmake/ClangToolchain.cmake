set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CLANG_SUFFIX -17 CACHE STRING "Suffix added to each llvm invocations")
set(CLANG_TRIPLE aarch64-linux-gnu CACHE STRING "Triple used by clang")

set(CMAKE_C_COMPILER_ID Clang)
set(CMAKE_CXX_COMPILER_ID Clang)

# Do no test the toolchain
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

set(CMAKE_ASM_COMPILER clang${CLANG_SUFFIX})
set(CMAKE_C_COMPILER clang${CLANG_SUFFIX})
set(CMAKE_CXX_COMPILER clang++${CLANG_SUFFIX})
set(CMAKE_OBJCOPY llvm-objcopy${CLANG_SUFFIX})

set(CMAKE_ASM_COMPILER_TARGET  ${CLANG_TRIPLE})
set(CMAKE_C_COMPILER_TARGET ${CLANG_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${CLANG_TRIPLE})

set(CMAKE_LINKER ld.lld${CLANG_SUFFIX})
set(CMAKE_CXX_LINK_EXECUTABLE  "<CMAKE_LINKER> -m aarch64elf <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# We fill the variable CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES according to our toolchain.
# This is needed according to https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_STANDARD_INCLUDE_DIRECTORIES.html
execute_process(
        COMMAND bash -c "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -x c++ -Wp,-v /dev/null 2>&1 > /dev/null | grep '^ /' | grep -w 'c++'"
        OUTPUT_VARIABLE COMPILER_HEADERS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX REPLACE "[ \n\t]+" ";" INCLUDE_COMPILER_HEADERS "${COMPILER_HEADERS}")
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${INCLUDE_COMPILER_HEADERS})

add_compile_options(-mno-unaligned-access)
