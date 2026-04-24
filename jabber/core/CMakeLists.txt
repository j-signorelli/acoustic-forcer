target_sources(jabber
    PRIVATE
        interpolant.cpp
        acoustic_field.cpp
        transfer_functions.cpp
        kernels.cpp
        psd.cpp
    PUBLIC
        FILE_SET core_headers TYPE HEADERS
        FILES
            interpolant.hpp
            acoustic_field.hpp
            transfer_functions.hpp
            kernels.hpp
            psd.hpp
            core.hpp)

if(JABBER_ENABLE_OPENMP)
    target_link_libraries(jabber PRIVATE OpenMP::OpenMP_CXX)
endif()
