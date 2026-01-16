find_package(Doxygen REQUIRED)
message(STATUS "Found Doxygen: TRUE")

configure_file(Doxyfile.in Doxyfile)

doxygen_add_docs(
    docs
    ${CMAKE_SOURCE_DIR}/src
    COMMENT "Generating project documentation."
    CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
)