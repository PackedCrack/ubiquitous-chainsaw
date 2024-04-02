if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory("windows")
else ()
    message(FATAL_ERROR "Operating systems other than windows are not yet implemented")
endif()


SET(BLUETOOTH_SRCS
        common.hpp
        Device.hpp
        CBLEScanner.hpp
)

target_sources(${MAIN_PROJECT} PRIVATE ${BLUETOOTH_SRCS})