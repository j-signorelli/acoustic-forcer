add_executable(unit-tests unit-tests.cpp)
target_link_libraries(unit-tests PRIVATE Catch2::Catch2WithMain
                                        libjabber)