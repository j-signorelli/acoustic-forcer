add_executable(unit-tests
                        test_compute_1d.cpp
                        test_compute_2d.cpp
                        test_compute_3d.cpp
                        test_app_common.cpp
                        test_app_config_input.cpp)

set(dependencies Catch2::Catch2WithMain libjabber)
if(JABBER_BUILD_APP)
    target_compile_definitions(unit-tests PRIVATE JABBER_WITH_APP)
    list(APPEND dependencies libjabber_app)
endif()

target_link_libraries(unit-tests PRIVATE ${dependencies})