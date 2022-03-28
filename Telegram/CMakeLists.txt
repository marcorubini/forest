cmake_minimum_required(VERSION 3.20.0)
project(
  forest-telegram
  VERSION 0.1
  LANGUAGES CXX)

add_library(forest-telegram INTERFACE)
add_library(forest::telegram ALIAS forest-telegram)
target_compile_features(forest-telegram INTERFACE cxx_std_20)
target_include_directories(forest-telegram INTERFACE include)
target_link_libraries(forest-telegram INTERFACE forest::machine forest::signal)

target_link_libraries(forest-telegram INTERFACE TgBot)
target_link_libraries(forest-telegram INTERFACE cereal::cereal)
target_link_libraries(forest-telegram INTERFACE spdlog::spdlog)

if(FOREST_BUILD_TESTS)
  add_subdirectory(tests)
endif()
