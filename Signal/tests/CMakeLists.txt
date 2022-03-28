create_test_sourcelist(Tests test_signal.cpp test_trivial.cpp)

add_executable(test_signal ${Tests})
target_link_libraries(test_signal forest::signal)

set(TestsToRun ${Tests})
remove(TestsToRun test_signal.cpp)

foreach(t ${TestsToRun})
  get_filename_component(TestName ${t} NAME_WE)
  add_test(NAME signal-${TestName} COMMAND test_signal ${TestName})
endforeach()
