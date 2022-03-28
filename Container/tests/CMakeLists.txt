create_test_sourcelist(Tests test_container.cpp test_trivial.cpp
                       test_string.cpp)

add_executable(test_container ${Tests})
target_link_libraries(test_container forest::container)

set(TestsToRun ${Tests})
remove(TestsToRun test_container.cpp)

foreach(t ${TestsToRun})
  get_filename_component(TestName ${t} NAME_WE)
  add_test(NAME container-${TestName} COMMAND test_container ${TestName})
endforeach()
