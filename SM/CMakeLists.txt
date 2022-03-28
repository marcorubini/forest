cmake_minimum_required(VERSION 3.20)
project(forest-sm VERSION 0.1)

include(cmake/CPM.cmake)

add_library(forest-sm INTERFACE)
add_library(forest::sm ALIAS forest-sm)
target_compile_features(forest-sm INTERFACE cxx_std_20)
target_include_directories(forest-sm INTERFACE include)

find_package(Boost 1.78.0 CONFIG)
if(NOT Boost_FOUND)
  cpmaddpackage("gh:boostorg/mp11#edba13573919a8a8fb8410360e3a3a0c18867c0b")
  set(FOREST_BOOST_TARGETS Boost::mp11)
else()
  set(FOREST_BOOST_TARGETS Boost::boost)
endif()

find_package(nlohmann_json 3.10.5 CONFIG)
if(NOT nlohmann_json_FOUND)
  cpmaddpackage("gh:nlohmann/json#4f8fba14066156b73f1189a2b8bd568bde5284c5")
endif()

find_package(SQLiteCpp 3.1.1 CONFIG)
if(NOT SQLiteCpp_FOUND)
  cpmaddpackage(
    "gh:SRombauts/SQLiteCpp#4e3d36af2d4a612d548f5959532a7b97c9724e6b")
endif()

find_package(TgBot 1.3 CONFIG)
if(NOT TgBot_FOUND)
  cpmaddpackage("gh:reo7sp/tgbot-cpp#f74c3812a58086aa2a62d8ecfaeee97f493c803e")
endif()

find_package(hfsm2 CONFIG)
if(NOT hfsm2_FOUND)
  cpmaddpackage(
    "gh:andrew-gresyk/HFSM2#c91d4e573760b6650cd7239c9e234478ca8c2d50")
endif()

target_link_libraries(forest-sm INTERFACE nlohmann_json::nlohmann_json)
target_link_libraries(forest-sm INTERFACE SQLiteCpp)
target_link_libraries(forest-sm INTERFACE TgBot)
target_link_libraries(forest-sm INTERFACE ${FOREST_BOOST_TARGETS})
target_include_directories(forest-sm INTERFACE ${HFSM2_SOURCE_DIR}/include)

add_subdirectory(test)
