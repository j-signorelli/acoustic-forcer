# Add jabber_app library
add_subdirectory(src)

# Build participant app
if (JABBER_BUILD_PARTICIPANT)
    # Find preCICE
    find_package(PRECICE 3.1.2 REQUIRED CONFIG NAMES precice)
    message(STATUS "Found preCICE config: TRUE")

    add_executable(jabber main.cpp)
    set_property(GLOBAL APPEND PROPERTY JABBER_TARGETS_PROPERTY jabber)
    
    target_link_libraries(jabber PRIVATE compiler_flag_requirements
                                    libjabber
                                    libjabber_app
                                    cxxopts::cxxopts
                                    precice::precice)
endif()
                                    