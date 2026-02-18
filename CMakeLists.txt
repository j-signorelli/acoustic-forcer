cmake_minimum_required(VERSION 3.13)

project(jabber LANGUAGES CXX VERSION 1.0.0)

# Create interface library for compiler flag requirements (requiring C++20)
add_library(compiler_flag_requirements INTERFACE)
target_compile_features(compiler_flag_requirements INTERFACE cxx_std_20)

# Main Jabber compiler options:
option(JABBER_BUILD_PARTICIPANT 
   "Build Jabber preCICE participant app (requires preCICE)." OFF)

option(JABBER_BUILD_VIZ 
   "Build Jabber flowfield visualizer app (requires MFEM)." OFF)

option(JABBER_BUILD_PROFILER 
   "Build Jabber compute profiler app." ON)

option(JABBER_BUILD_OUT 
   "Build Jabber output app." ON)

option(JABBER_BUILD_PLOT 
   "Build Jabber plot app." ON)

option(JABBER_BUILD_PSD
   "Build Jabber PSD app." ON)

option(JABBER_BUILD_DOCS 
   "Build Jabber documentation w/ Doxygen." OFF)

option(JABBER_BUILD_APP_LIB 
   "Build the jabber_app library. Required to build apps." ON)

option(JABBER_BUILD_TESTS 
   "Build unit tests" ON)

option(JABBER_ENABLE_OPENMP
   "Build jabber with OpenMP support" OFF)

# Property of all targets to install
set_property(GLOBAL PROPERTY JABBER_TARGETS_PROPERTY)

if (JABBER_ENABLE_OPENMP)
   find_package(OpenMP REQUIRED COMPONENTS CXX)
   add_compile_definitions(JABBER_WITH_OPENMP)
endif()

# Build Jabber library
add_subdirectory(src/core)

# If building any apps
if (JABBER_BUILD_APP_LIB)
   add_compile_definitions(JABBER_WITH_APP)

   # Add TOML11 -- required for app library
   add_subdirectory(externals/toml11)

   add_subdirectory(src/app)
endif()

# If building apps
if (JABBER_BUILD_PARTICIPANT 
      OR JABBER_BUILD_VIZ 
      OR JABBER_BUILD_PROFILER
      OR JABBER_BUILD_OUT 
      OR JABBER_BUILD_PLOT
      OR JABBER_BUILD_PSD)

   # All apps use cxxopts
   add_subdirectory(externals/cxxopts)

   # Add app subdirectory
   add_subdirectory(apps)
   
endif()

if (JABBER_BUILD_DOCS)
   add_subdirectory(docs)
endif()

if (JABBER_BUILD_TESTS)
   add_subdirectory(externals/Catch2)
   add_subdirectory(tests)
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
   FILE_SET jabber_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jabber
   FILE_SET jabber_app_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jabber_app)
