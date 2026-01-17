# Build jabber executable
add_executable(jabber config_input.cpp
                      main.cpp)
set_property(GLOBAL APPEND PROPERTY JABBER_TARGETS_PROPERTY jabber)

target_link_libraries(jabber PRIVATE libjabber
                                    cxxopts::cxxopts
                                    precice::precice
                                    toml11::toml11)
                                    