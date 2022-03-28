cmake_minimum_required(VERSION 3.20.0)
project(
  forest-signal
  VERSION 0.1
  LANGUAGES CXX)

add_library(forest-signal INTERFACE)
add_library(forest::signal ALIAS forest-signal)
target_compile_features(forest-signal INTERFACE cxx_std_20)
target_include_directories(forest-signal INTERFACE include)
target_link_libraries(forest-signal INTERFACE forest::meta
                                              nlohmann_json::nlohmann_json)

if(FOREST_BUILD_TESTS)
  add_subdirectory(tests)
endif()
