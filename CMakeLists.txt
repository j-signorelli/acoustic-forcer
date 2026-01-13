cmake_minimum_required(VERSION 3.10)

project(acoustic_participant LANGUAGES CXX)

# Create interface library for compiler flag requirements (requiring C++20)
add_library(compiler_flag_requirements INTERFACE)
target_compile_features(compiler_flag_requirements INTERFACE cxx_std_20)

# Find preCICE
find_package(PRECICE 3.1.2 REQUIRED CONFIG NAMES precice)
message(STATUS "Found preCICE config: TRUE")

add_subdirectory(externals/cxxopts)


add_executable(acoustic_participant run.cpp)
target_link_libraries(acoustic_participant PUBLIC 
                        compiler_flag_requirements
                        precice::precice
                        cxxopts::cxxopts)

message(STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")

# Install
install(TARGETS acoustic_participant DESTINATION bin)