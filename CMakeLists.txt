cmake_minimum_required(VERSION 3.13)

project(jabber LANGUAGES CXX VERSION 1.0.0)

# Create interface library for compiler flag requirements (requiring C++20)
add_library(compiler_flag_requirements INTERFACE)
target_compile_features(compiler_flag_requirements INTERFACE cxx_std_20)

# Find preCICE
find_package(PRECICE 3.1.2 REQUIRED CONFIG NAMES precice)
message(STATUS "Found preCICE config: TRUE")

# Add provided external dependencies
add_subdirectory(externals/cxxopts)
add_subdirectory(externals/toml11)
add_subdirectory(src)

message(STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")

add_executable(jabber run.cpp)
target_link_libraries(jabber 
   PRIVATE libjabber cxxopts::cxxopts precice::precice)

# Install
# From: https://stackoverflow.com/questions/17511496/how-to-create-a-shared-library-with-cmake
include(GNUInstallDirs)
install(TARGETS libjabber jabber
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jabber)
#install(TARGETS jabber DESTINATION bin)