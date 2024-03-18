add_library(libk STATIC
    src/memory.cpp
    src/string.cpp

    src/optimized/aarch64/memcpy-sve.S
    src/optimized/aarch64/memcpy.S
    src/optimized/aarch64/memset.S
    src/optimized/aarch64/strlen-sve.S
    src/optimized/aarch64/strlen.S
    src/optimized/aarch64/strcpy-sve.S
    src/optimized/aarch64/strcpy.S
)

target_compile_definitions(libk PUBLIC -DLIBK_ENABLE_C=1)
target_include_directories(libk PUBLIC include)
