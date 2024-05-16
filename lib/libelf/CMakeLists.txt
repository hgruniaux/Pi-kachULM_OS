add_library(libelf STATIC
        include/elf/elf.hpp
        src/elf.cpp)

target_include_directories(libelf PUBLIC include/)
