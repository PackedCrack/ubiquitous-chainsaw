if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_subdirectory("windows")
else ()
    message(FATAL_ERROR "Operating systems other than windows are not yet implemented")
endif()


SET(BLUETOOTH_SRCS
        ble_common.hpp
        Device.hpp
        Service.hpp
        Characteristic.hpp
        Descriptor.hpp
        Scanner.hpp
)

target_sources(${MAIN_PROJECT} PRIVATE ${BLUETOOTH_SRCS})