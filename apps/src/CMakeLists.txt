# Create jabber_app library
add_library(libjabber_app)

# CMake automatically preprends "lib", so ensure that it is just "libjabber"
set_target_properties(libjabber_app PROPERTIES OUTPUT_NAME "jabber_app")

target_sources(libjabber_app
    PRIVATE 
        config_input.cpp
    PUBLIC
        FILE_SET HEADERS
        FILES
            params.hpp
            config_input.hpp
            jabber_app.hpp)

target_link_libraries(libjabber_app PUBLIC compiler_flag_requirements
                                            toml11::toml11)