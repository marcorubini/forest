cmake_minimum_required(VERSION 3.20)
project(forest-tsm VERSION 0.1)

add_library(forest-tsm INTERFACE)
add_library(forest::tsm ALIAS forest-tsm)
target_compile_features(forest-tsm INTERFACE cxx_std_20)
target_include_directories(forest-tsm INTERFACE include)
target_link_libraries(forest-tsm INTERFACE forest::sm)

add_subdirectory(test)