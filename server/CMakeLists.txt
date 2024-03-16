# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)


include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(../cmake/CompilerWarnings.cmake)


SET(MAIN_PROJECT esp32-server)
project(${MAIN_PROJECT} LANGUAGES C CXX)


# Can't configure cmake because its hijacked by espressif......
# Have to resort to globals
add_compile_options("-std=c++20" "${GCC_WARNINGS}")