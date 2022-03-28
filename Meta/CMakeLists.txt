cmake_minimum_required(VERSION 3.20.0)
project(
  forest-meta
  VERSION 0.1
  LANGUAGES CXX)

add_library(forest-meta INTERFACE)
add_library(forest::meta ALIAS forest-meta)
target_compile_features(forest-meta INTERFACE cxx_std_20)
target_include_directories(forest-meta INTERFACE include)

if(FOREST_BUILD_TESTS)
  add_subdirectory(tests)
endif()
