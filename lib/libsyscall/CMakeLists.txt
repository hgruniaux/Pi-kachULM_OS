add_library(libsyscall STATIC
        include/syscall/syscall.h
        include/syscall/message.h
        src/message.c
        src/message.S
        src/syscall.c
        src/syscall.S)

target_include_directories(libsyscall PUBLIC include/)
