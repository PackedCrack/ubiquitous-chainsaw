if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory("windows")
else()
    message(FATAL_ERROR "Only windows is implemented as of now")
endif()

SET(SYSTEM_SRCS
        System.hpp
        TrayIcon.hpp)

target_sources(${MAIN_PROJECT} PRIVATE ${SYSTEM_SRCS})