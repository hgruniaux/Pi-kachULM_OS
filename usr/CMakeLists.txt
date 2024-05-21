function(add_userspace_executable name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE libsyscall)
    target_compile_options(${name} PRIVATE -nostdlib -no-pie -Wl,-e_start)
    target_link_options(${name} PRIVATE -nostdlib -no-pie -Wl,-e_start)
endfunction()

add_userspace_executable(init init.c)
add_userspace_executable(credits credits.c)
