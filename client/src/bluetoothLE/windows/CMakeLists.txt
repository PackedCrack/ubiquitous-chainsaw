# TODO: Checks that we are configuring for windows

SET(WINDOWS_BLE_SRCS
    CScanner.cpp
        CDevice.hpp
        CDevice.cpp
        CService.cpp
        CCharacteristic.cpp
        CDescriptor.cpp)

target_sources(${MAIN_PROJECT} PRIVATE ${WINDOWS_BLE_SRCS})