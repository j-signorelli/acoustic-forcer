# Add sources to jabber library
target_sources(jabber
   PRIVATE 
      config_input.cpp
      common.cpp
   PUBLIC
      FILE_SET app_headers TYPE HEADERS
      FILES
         params.hpp
         config_input.hpp
         common.hpp
         app.hpp)

target_link_libraries(jabber PRIVATE toml11::toml11)                                      