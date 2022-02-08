cmake_minimum_required(VERSION 3.20)
project(forest-sm VERSION 0.1)

add_library(forest-sm INTERFACE)
add_library(forest::sm ALIAS forest-sm)
target_compile_features(forest-sm INTERFACE cxx_std_20)
target_include_directories(forest-sm INTERFACE include)

add_subdirectory(test)
