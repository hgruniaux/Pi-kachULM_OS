add_library(libsyscall STATIC
        include/sys/__syscall.h
        include/sys/__types.h
        include/sys/__utils.h
        include/sys/syscall.h
        include/sys/syscall_table.h
        include/sys/window.h
        include/string.h

        src/sys/window.c
        src/sys/syscall.c

        src/string/strlen.c
        src/string/memcmp.c
        src/string/memset.c
        src/string/memcpy.c
        src/string/memmove.c
        include/stdlib.h
        src/stdlib/malloc_free.c
        include/assert.h
        src/stdlib/assert.c
        src/stdlib/rand.c)

target_include_directories(libsyscall PUBLIC include/)
