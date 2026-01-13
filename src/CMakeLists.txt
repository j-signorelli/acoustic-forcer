# Create jabber library
add_library(libjabber)

target_sources(libjabber
    PRIVATE 
        config.cpp
        acoustic_field.cpp
    PUBLIC
        FILE_SET HEADERS
        FILES
            config.hpp
            acoustic_field.hpp)

# CMake automatically preprends "lib", so ensure that it is just "libjabber"
set_target_properties(libjabber PROPERTIES OUTPUT_NAME "jabber")

target_link_libraries(libjabber
    PRIVATE toml11::toml11
    PUBLIC  compiler_flag_requirements)
