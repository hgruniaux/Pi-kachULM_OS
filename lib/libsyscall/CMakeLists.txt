add_library(libsyscall STATIC
        include/sys/__syscall.h
        include/sys/__types.h
        include/sys/__utils.h
        include/sys/syscall.h
        include/sys/syscall_table.h
        include/sys/window.h
        include/sys/keyboard.h

        include/string.h
        include/stdlib.h
        include/assert.h

        src/sys/window.c
        src/sys/syscall.c
        src/sys/keyboard.c
        src/sys/args.c
        src/sys/pipe.c

        src/string/strlen.c
        src/string/memcmp.c
        src/string/memset.c
        src/string/memcpy.c
        src/string/memmove.c

        src/stdlib/malloc_free.c
        src/stdlib/assert.c
        src/stdlib/rand.c
        src/startup.c
        include/sys/file.h
        src/sys/file.c)

target_include_directories(libsyscall PUBLIC include/)
