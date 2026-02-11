# https://stackoverflow.com/a/66989192
include(CheckIncludeFileCXX)
check_include_file_cxx("format" CXX_FORMAT_SUPPORT)
if (NOT CXX_FORMAT_SUPPORT)
    find_package(fmt)
    if (NOT fmt_FOUND)
        include(FetchContent)
        FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281) # 10.2.1
        FetchContent_MakeAvailable(fmt)
    endif()
    target_link_libraries(compiler_flag_requirements INTERFACE fmt::fmt)
    add_compile_definitions(USE_FMTLIB_POLYFILL)
endif()

configure_file(format.hpp.in format.hpp)

target_include_directories(compiler_flag_requirements INTERFACE 
                            ${CMAKE_CURRENT_BINARY_DIR})