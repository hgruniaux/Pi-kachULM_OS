add_library(device-tree STATIC
        include/dtb/dtb.hpp
        include/dtb/reserved_sections.hpp
        include/dtb/node.hpp
        include/dtb/parser.hpp

        src/dtb.cpp
        src/reserved_sections.cpp
        src/node.cpp
        src/parser.cpp
        src/utils.hpp
)

target_include_directories(device-tree PUBLIC include/)

target_link_libraries(device-tree PRIVATE libk)
