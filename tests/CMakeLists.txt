add_executable(tests unit-tests.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2)