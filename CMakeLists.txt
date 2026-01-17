cmake_minimum_required(VERSION 3.13)

project(jabber LANGUAGES CXX VERSION 1.0.0)

# Create interface library for compiler flag requirements (requiring C++20)
add_library(compiler_flag_requirements INTERFACE)
target_compile_features(compiler_flag_requirements INTERFACE cxx_std_20)

# Property of all targets to install
set_property(GLOBAL PROPERTY JABBER_TARGETS_PROPERTY)

set(DOC_PATHS ${CMAKE_SOURCE_DIR}/src)

# Build Jabber library
add_subdirectory(src)

# Build Jabber app, if ON
option(JABBER_BUILD_APP "Build Jabber executable." ON)
if (JABBER_BUILD_APP)

    # Find preCICE
    find_package(PRECICE 3.1.2 REQUIRED CONFIG NAMES precice)
    message(STATUS "Found preCICE config: TRUE")

    # Add external dependencies - required for app
    add_subdirectory(externals/cxxopts)
    add_subdirectory(externals/toml11)
    
    # Add app subdirectory
    add_subdirectory(app)

    list(APPEND DOC_PATHS ${CMAKE_SOURCE_DIR}/app)
endif()

# Build Jabber documentation, if ON
option(JABBER_BUILD_DOCS "Build Jabber documentation w/ Doxygen." OFF)
if (JABBER_BUILD_DOCS)
    add_subdirectory(docs)
endif()


message(STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")

get_property(JABBER_TARGETS GLOBAL PROPERTY JABBER_TARGETS_PROPERTY)

# Install
# From: https://stackoverflow.com/questions/17511496/how-to-create-a-shared-library-with-cmake
include(GNUInstallDirs)
install(TARGETS ${JABBER_TARGETS}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jabber)