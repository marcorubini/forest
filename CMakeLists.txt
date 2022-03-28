cmake_minimum_required(VERSION 3.20.0)
project(forest)

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(FOREST_MASTER_PROJECT ON)
endif()

option(FOREST_BUILD_TESTS "Build tests" ${FOREST_MASTER_PROJECT})

if(FOREST_BUILD_TESTS)
  enable_testing()
endif()

include(cmake/CPM.cmake)
include(cmake/dependencies.cmake)

add_subdirectory(Meta)
add_subdirectory(Container)
add_subdirectory(Machine)
add_subdirectory(Signal)
add_subdirectory(Telegram)

# add_subdirectory(SM)

add_library(forest INTERFACE)
add_library(forest::forest ALIAS forest)
# target_link_libraries(forest INTERFACE forest::sm)
