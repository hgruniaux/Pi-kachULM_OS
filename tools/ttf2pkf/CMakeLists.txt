cmake_minimum_required(VERSION 3.16)

project(ttf2pkf)

find_package(Freetype REQUIRED)
add_executable(ttf2pkf main.cpp)
target_link_libraries(ttf2pkf PRIVATE Freetype::Freetype)
