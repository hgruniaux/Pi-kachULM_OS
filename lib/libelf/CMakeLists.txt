add_library(elf STATIC
        include/elf/elf.hpp
        src/elf.cpp)

target_include_directories(elf PUBLIC include)
