# Create jabber library
add_library(libjabber)
set_property(GLOBAL APPEND PROPERTY JABBER_TARGETS_PROPERTY libjabber)

# CMake automatically preprends "lib", so ensure that it is just "libjabber"
set_target_properties(libjabber PROPERTIES OUTPUT_NAME "jabber")

target_sources(libjabber
    PRIVATE 
        acoustic_field.cpp
        transfer_functions.cpp
    PUBLIC
        FILE_SET HEADERS
        FILES
            acoustic_field.hpp
            transfer_functions.hpp
            jabber.hpp)

target_link_libraries(libjabber PUBLIC compiler_flag_requirements)
