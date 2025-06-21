cmake_minimum_required(VERSION 3.16)
project(budiv)

set(CMAKE_CXX_STANDARD 17)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_subdirectory(vendor/raylib)
add_subdirectory(lang)

