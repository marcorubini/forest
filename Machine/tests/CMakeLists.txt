create_test_sourcelist(Tests test_machine.cpp test_trivial.cpp
                       test_enter_exit.cpp test_reenter.cpp test_region.cpp)

add_executable(test_machine ${Tests})
target_link_libraries(test_machine forest::machine)

set(TestsToRun ${Tests})
remove(TestsToRun test_machine.cpp)

foreach(t ${TestsToRun})
  get_filename_component(TestName ${t} NAME_WE)
  add_test(NAME machine-${TestName} COMMAND test_machine ${TestName})
endforeach()
