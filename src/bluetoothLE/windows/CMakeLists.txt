# TODO: Checks that we are configuring for windows

SET(WINDOWS_BLE_SRCS
    CScanner.cpp
    CScanner.hpp)

target_sources(${MAIN_PROJECT} PRIVATE ${WINDOWS_BLE_SRCS})