add_subdirectory(src/optimized)
#add_subdirectory(src/ryu)

add_library(libk STATIC
        src/format.cpp
        src/format_impl.cpp
        src/format_impl.hpp
        src/string.cpp

        include/libk/assert.hpp
        include/libk/format.hpp
        include/libk/spinlock.hpp
        include/libk/string.hpp
        include/libk/utils.hpp
)

target_include_directories(libk PUBLIC include/)

target_link_libraries(libk PRIVATE optimized-routines)
#target_link_libraries(libk PRIVATE ryu-algorithm)

# Define LIBK_ENABLE_C so we export the C stdlib functions.
target_compile_definitions(libk PUBLIC -DLIBK_ENABLE_C=1)

# Use re2c to generate the format lexer. This is only required if
# the lexer specification (in format.re2c) change. So, it is a soft
# dependency.
find_program(RE2C_PROGRAM re2c)
if (RE2C_PROGRAM)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/format.cpp
            COMMAND ${RE2C_PROGRAM}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/format.re2c
            -o ${CMAKE_CURRENT_SOURCE_DIR}/src/format.cpp
            -W # enable all warnings
            -Werror # turn warnings into errors
            --case-ranges
            --computed-gotos
            # The following three options are used to avoid generating big diffs on small changes for Git.
            --no-debug-info
            --no-generation-date
            --no-version
            WORKING_DIRECTORY .
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/format.re2c
            COMMENT "Generating libk::format() lexer using re2c"
    )
endif ()
