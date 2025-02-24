set(RAMFS_DIR "${CMAKE_SOURCE_DIR}/fs/")
set(RAMFS_BIN_DIR "${RAMFS_DIR}/bin")
set(CREATE_FS_SCRIPT "${CMAKE_SOURCE_DIR}/tools/create-fs.sh")

add_custom_target(_create_ramfs_bin_dir COMMAND ${CMAKE_COMMAND} -E make_directory "${RAMFS_BIN_DIR}")

set(exec-deps)

# Create the standard runtime C++ static library
add_library(libcxx STATIC
  libcxx/alloc.cpp
  )
target_link_libraries(libcxx PRIVATE libsyscall)
target_compile_options(libcxx PRIVATE -nostdlib -no-pie)
target_link_options(libcxx PRIVATE -nostdlib -no-pie)
target_include_directories(libcxx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libcxx)

# Create the UI Tulip static library
add_library(tulip STATIC
  tulip/application.hpp
  tulip/application.cpp
  tulip/button.hpp
  tulip/button.cpp
  tulip/event.hpp
  tulip/layout.hpp
  tulip/layout.cpp
  tulip/painter.hpp
  tulip/painter.cpp
  tulip/style.hpp
  tulip/style.cpp
  tulip/title_bar.hpp
  tulip/title_bar.cpp
  tulip/utils.hpp
  tulip/vector.hpp
  tulip/widget.hpp
  tulip/widget.cpp
  tulip/window.hpp
  tulip/window.cpp
  )
target_link_libraries(tulip PRIVATE libsyscall libcxx)
target_compile_options(tulip PRIVATE -nostdlib -no-pie)
target_link_options(tulip PRIVATE -nostdlib -no-pie)
target_include_directories(tulip PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Helper Macro to Add Userspace executables
macro(add_userspace_executable name)
    add_executable("${name}" ${ARGN})
    target_link_libraries("${name}" PRIVATE libsyscall)
    target_compile_options("${name}" PRIVATE -nostdlib -no-pie -Wl,-e_start)
    target_link_options("${name}" PRIVATE -nostdlib -no-pie -e _start)

    list(APPEND exec-deps "${RAMFS_BIN_DIR}/${name}")
    add_custom_command(
            OUTPUT "${RAMFS_BIN_DIR}/${name}"
            DEPENDS _create_ramfs_bin_dir "${name}"
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${name} "${RAMFS_BIN_DIR}")
endmacro()

# List Userspace Executables Here
add_userspace_executable(init init.c)
add_userspace_executable(credits credits.c)
add_userspace_executable(slides slides.c stb_image.c)
add_userspace_executable(explorer explorer.c)
add_userspace_executable(image_viewer image_viewer.c stb_image.c)
add_userspace_executable(text_viewer text_viewer.c)
add_userspace_executable(test_pipe test_pipe.c)
add_userspace_executable(test_ui test_ui.cpp)
target_link_libraries(test_ui PRIVATE tulip libcxx)

# The File System Will be in (your build dir)/binuser/fs.img

add_custom_target(_create_fs_img
        DEPENDS ${exec-deps}
        COMMAND sh "${CREATE_FS_SCRIPT}" "${CMAKE_CURRENT_BINARY_DIR}/fs.img")

add_custom_target(_clean_ramfs_bin_dir
        DEPENDS _create_fs_img
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${RAMFS_BIN_DIR}")

add_custom_target(fs-img
        DEPENDS _clean_ramfs_bin_dir)
