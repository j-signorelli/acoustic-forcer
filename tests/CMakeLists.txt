add_executable(unit-tests
                        test_acoustic_field.cpp
                        test_single_wave_1d.cpp)
target_link_libraries(unit-tests PRIVATE Catch2::Catch2WithMain
                                        libjabber)