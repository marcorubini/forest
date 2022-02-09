cmake_minimum_required(VERSION 3.20)
project(forest VERSION 0.1)

add_library(forest INTERFACE)
add_library(forest::forest ALIAS forest)

add_subdirectory(SM)
add_subdirectory(TSM)

target_link_libraries(forest INTERFACE forest::sm)
target_link_libraries(forest INTERFACE forest::tsm)