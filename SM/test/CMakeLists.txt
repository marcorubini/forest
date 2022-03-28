add_executable(00-trivial 00-trivial.cpp)
target_link_libraries(00-trivial forest::sm)

# add_executable(01-demo 01-demo.cpp) target_link_libraries(01-demo forest::sm)

# add_executable(02-timeouts 02-timeouts.cpp) target_link_libraries(02-timeouts
# forest::sm)

add_executable(02-sm 02-sm.cpp)
target_link_libraries(02-sm forest::sm)
