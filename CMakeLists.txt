cmake_minimum_required(VERSION 3.20.0)
project(
  forest
  VERSION 0.1
  DESCRIPTION "Framework for developing telegram bots"
  HOMEPAGE_URL "TODO"
  LANGUAGES CXX)

include(FetchContent)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

# set(BANANA_ENABLE_CORO TRUE)
#
# set(BANANA_USE_BUNDLED_CPR TRUE)

FetchContent_Declare(banana GIT_REPOSITORY "https://github.com/Smertig/banana")
FetchContent_MakeAvailable(banana)

FetchContent_Declare(cpr GIT_REPOSITORY "https://github.com/libcpr/cpr.git")
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(nlohmann_json GIT_REPOSITORY "https://github.com/nlohmann/json.git")
FetchContent_MakeAvailable(nlohmann_json)

set(SQLITECPP_RUN_CPPCHECK
    OFF
    CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT
    OFF
    CACHE BOOL "" FORCE)
set(SQLITECPP_INTERNAL_SQLITE
    OFF
    CACHE BOOL "" FORCE)

# FetchContent_Declare(SQLiteCpp GIT_REPOSITORY "https://github.com/SRombauts/SQLiteCpp.git")
# FetchContent_MakeAvailable(SQLiteCpp)

find_package(SQLite3 CONFIG REQUIRED)
find_package(sqlitecpp CONFIG REQUIRED)

add_library(forest INTERFACE)
add_library(forest::forest ALIAS forest)
target_compile_features(forest INTERFACE cxx_std_20)
target_include_directories(forest INTERFACE include)
target_link_libraries(forest INTERFACE banana banana-cpr cpr::cpr nlohmann_json::nlohmann_json SQLiteCpp
                                       SQLite::SQLite3)

add_subdirectory(test)
