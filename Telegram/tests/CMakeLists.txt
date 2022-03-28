add_executable(test_telegram driver.cpp)
target_link_libraries(test_telegram PRIVATE doctest::doctest forest::telegram)
target_compile_definitions(test_telegram
                           PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)

add_executable(test_telegram_demo1 test_demo1.cpp)
target_link_libraries(test_telegram_demo1 PRIVATE forest::telegram)

add_executable(test_telegram_demo2 test_demo2.cpp)
target_link_libraries(test_telegram_demo2 PRIVATE forest::telegram)
