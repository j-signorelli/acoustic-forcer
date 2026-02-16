# Create jabber_app library
add_library(libjabber_app)

# CMake automatically preprends "lib", so ensure that it is just "libjabber"
set_target_properties(libjabber_app PROPERTIES OUTPUT_NAME "jabber_app")

target_sources(libjabber_app
   PRIVATE 
      config_input.cpp
      common.cpp
   PUBLIC
      FILE_SET jabber_app_headers TYPE HEADERS
      FILES
         params.hpp
         config_input.hpp
         common.hpp
         jabber_app.hpp)

target_link_libraries(libjabber_app PUBLIC compiler_flag_requirements
                                          libjabber
                                    PRIVATE
                                          toml11::toml11)
                                          
set_property(GLOBAL APPEND PROPERTY JABBER_TARGETS_PROPERTY 
                                                libjabber_app)                                          