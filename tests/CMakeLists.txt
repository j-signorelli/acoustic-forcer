add_executable(unit-tests
                        test_1d.cpp)
target_link_libraries(unit-tests PRIVATE Catch2::Catch2WithMain
                                        libjabber)