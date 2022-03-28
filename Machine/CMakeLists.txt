cmake_minimum_required(VERSION 3.20.0)
project(
  forest-machine
  VERSION 0.1
  LANGUAGES CXX)

add_library(forest-machine INTERFACE)
add_library(forest::machine ALIAS forest-machine)
target_compile_features(forest-machine INTERFACE cxx_std_20)
target_include_directories(forest-machine INTERFACE include)
target_link_libraries(forest-machine INTERFACE forest::container
                                               ${BOOST_MP11_TARGET})

if(FOREST_BUILD_TESTS)
  add_subdirectory(tests)
endif()
