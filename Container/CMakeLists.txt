cmake_minimum_required(VERSION 3.20.0)
project(
  forest-container
  VERSION 0.1
  LANGUAGES CXX)

add_library(forest-container INTERFACE)
add_library(forest::container ALIAS forest-container)
target_compile_features(forest-container INTERFACE cxx_std_20)
target_include_directories(forest-container INTERFACE include)
target_link_libraries(forest-container INTERFACE forest::meta)

if(FOREST_BUILD_TESTS)
  add_subdirectory(tests)
endif()
