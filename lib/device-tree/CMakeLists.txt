add_library(device-tree STATIC
        dtb/dtb.hpp
        dtb.cpp

        dtb/node.hpp
        node.cpp

        dtb/parser.hpp
        parser.cpp

        utils.hpp
)

target_include_directories(device-tree PUBLIC .)

target_link_libraries(device-tree PRIVATE libk)
