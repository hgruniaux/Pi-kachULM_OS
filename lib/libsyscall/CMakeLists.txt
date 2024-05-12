add_library(libsyscall STATIC
        include/syscall/syscall.h
        src/syscall.c
        src/syscall.S)

target_include_directories(libsyscall PUBLIC include/)
