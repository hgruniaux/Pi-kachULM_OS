add_library(ryu-algorithm STATIC
        d2fixed.cpp
        d2s.cpp
        f2s.cpp
)

target_include_directories(ryu-algorithm PRIVATE ${CMAKE_SOURCE_DIR}/libk/include)

# Disable clang-tidy on ryu. The code comes from outside, we don't control it.
set_target_properties(ryu-algorithm PROPERTIES CXX_CLANG_TIDY "")
