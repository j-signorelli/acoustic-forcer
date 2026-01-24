add_executable(unit-tests
                        test_1d.cpp)

set(dependencies Catch2::Catch2WithMain libjabber)
if(JABBER_BUILD_APP)
    target_compile_definitions(unit-tests PRIVATE JABBER_WITH_APP)
    list(APPEND dependencies libjabber_app)
endif()

target_link_libraries(unit-tests PRIVATE ${dependencies})