add_library(libk STATIC
        src/format.cpp
        src/format_impl.cpp
        src/format_impl.hpp
        src/string.cpp
        src/log.cpp
        src/assert.cpp
        src/test.cpp
        src/bit_array.cpp
        src/linear_allocator.cpp
        src/qemu.cpp

        include/libk/assert.hpp
        include/libk/format.hpp
        include/libk/string.hpp
        include/libk/utils.hpp
        include/libk/bit_array.hpp
        include/libk/option.hpp
        include/libk/string_view.hpp
        include/libk/log.hpp
        include/libk/linear_allocator.hpp
        include/libk/hash.hpp
        include/libk/test.hpp
        include/libk/linked_list.hpp
        include/libk/qemu.hpp
)

target_include_directories(libk PUBLIC include/)

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
