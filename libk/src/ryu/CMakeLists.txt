add_library(ryu-algorithm STATIC
        d2fixed.cpp
        d2s.cpp
        f2s.cpp
)

target_include_directories(ryu-algorithm PRIVATE ${CMAKE_SOURCE_DIR}/libk/include)
