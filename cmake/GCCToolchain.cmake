set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(GCC_PREFIX aarch64-linux-gnu- CACHE STRING "Prefix added to each GNU binutils and GCC invocations")
set(GCC_SUFFIX -13 CACHE STRING "Suffix added to each GNU binutils and GCC invocations")
set(CMAKE_ASM_COMPILER ${GCC_PREFIX}gcc${GCC_SUFFIX})
set(CMAKE_C_COMPILER ${GCC_PREFIX}gcc${GCC_SUFFIX})
set(CMAKE_CXX_COMPILER ${GCC_PREFIX}g++${GCC_SUFFIX})
set(CMAKE_OBJCOPY ${GCC_PREFIX}objcopy)

set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

# Do no test the toolchain
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

set(CMAKE_LINKER ${GCC_PREFIX}ld)
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_LINKER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# We fill the variable CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES according to our toolchain.
# This is needed according to https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_STANDARD_INCLUDE_DIRECTORIES.html
execute_process(
        COMMAND bash -c "${CMAKE_CXX_COMPILER} -x c++ -Wp,-v /dev/null 2>&1 > /dev/null | grep '^ /' | grep -w 'c++'"
        OUTPUT_VARIABLE COMPILER_HEADERS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX REPLACE "[ \n\t]+" ";" INCLUDE_COMPILER_HEADERS "${COMPILER_HEADERS}")
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${INCLUDE_COMPILER_HEADERS})
