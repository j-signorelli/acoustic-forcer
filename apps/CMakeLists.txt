# Add jabber_app library
add_subdirectory(src)

# Build participant app
if (JABBER_BUILD_PARTICIPANT)
   # Find preCICE
   find_package(PRECICE 3.1.2 REQUIRED CONFIG NAMES precice)
   message(STATUS "Found preCICE config: TRUE")

   add_executable(jabber_participant participant.cpp)
   set_property(GLOBAL APPEND PROPERTY JABBER_TARGETS_PROPERTY 
                                                   jabber_participant)

   target_link_libraries(jabber_participant PRIVATE 
                                 compiler_flag_requirements
                                 libjabber
                                 libjabber_app
                                 cxxopts::cxxopts
                                 precice::precice)
endif()


# Build Jabber acoustic field visualizer app
if (JABBER_BUILD_VIZ)

   # Find MFEM
   find_package(MFEM 4.8 REQUIRED CONFIG)
   message(STATUS "Found MFEM config: TRUE")

   add_executable(jabber_viz viz.cpp)
   set_property(GLOBAL APPEND PROPERTY JABBER_TARGETS_PROPERTY 
                                                   jabber_viz)

   target_link_libraries(jabber_viz PRIVATE 
                                 compiler_flag_requirements
                                 libjabber
                                 libjabber_app
                                 cxxopts::cxxopts
                                 mfem)

endif()


# Build Jabber profiler app
if (JABBER_BUILD_PROFILER)

   add_executable(jabber_profile profile.cpp)
   set_property(GLOBAL APPEND PROPERTY JABBER_TARGETS_PROPERTY 
                                                   jabber_profile)

   target_link_libraries(jabber_profile PRIVATE 
                                       compiler_flag_requirements
                                       libjabber
                                       libjabber_app
                                       cxxopts::cxxopts)

endif()