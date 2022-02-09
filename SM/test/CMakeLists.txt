add_executable(00-trivial 00-trivial.cpp)
target_link_libraries(00-trivial forest::sm)

add_executable(01-transition_table 01-transition_table.cpp)
target_link_libraries(01-transition_table forest::sm)

add_executable(02-timeouts 02-timeouts.cpp)
target_link_libraries(02-timeouts forest::sm)