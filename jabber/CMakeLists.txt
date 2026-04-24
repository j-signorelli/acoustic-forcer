# Create jabber library
add_library(jabber)
target_compile_features(jabber PUBLIC cxx_std_20)
set_property(GLOBAL APPEND PROPERTY JABBER_TARGETS_PROPERTY jabber)

target_sources(jabber PUBLIC FILE_SET main_header TYPE HEADERS FILES jabber.hpp)

add_subdirectory(core)

if (JABBER_BUILD_APP_LIB)
    add_subdirectory(app)
endif()