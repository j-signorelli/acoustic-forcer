# Create jabber library
add_library(libjabber)
set_property(GLOBAL APPEND PROPERTY JABBER_TARGETS_PROPERTY libjabber)

# CMake automatically preprends "lib", so ensure that it is just "libjabber"
set_target_properties(libjabber PROPERTIES OUTPUT_NAME "jabber")

target_sources(libjabber
    PRIVATE
        interpolant.cpp
        acoustic_field.cpp
        transfer_functions.cpp
        kernels.cpp
        psd.cpp
    PUBLIC
        FILE_SET jabber_headers TYPE HEADERS
        FILES
            interpolant.hpp
            acoustic_field.hpp
            transfer_functions.hpp
            kernels.hpp
            psd.hpp
            jabber.hpp)

target_link_libraries(libjabber PUBLIC compiler_flag_requirements)
if(JABBER_ENABLE_OPENMP)
    target_link_libraries(libjabber PUBLIC OpenMP::OpenMP_CXX)
endif()
