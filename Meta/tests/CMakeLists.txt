create_test_sourcelist(Tests test_meta.cpp test_trivial.cpp)

add_executable(test_meta ${Tests})
target_link_libraries(test_meta forest::meta)

set(TestsToRun ${Tests})
remove(TestsToRun test_meta.cpp)

foreach(t ${TestsToRun})
  get_filename_component(TestName ${t} NAME_WE)
  add_test(NAME meta-${TestName} COMMAND test_meta ${TestName})
endforeach()
