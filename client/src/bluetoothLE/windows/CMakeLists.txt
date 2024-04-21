# TODO: Checks that we are configuring for windows

SET(WINDOWS_BLE_SRCS
		win_ble_common.hpp
	CScanner.cpp
	CScanner.hpp
	CDevice.cpp
        CDevice.hpp
        CService.cpp
		CService.hpp
        CCharacteristic.cpp
		CCharacteristic.hpp
        CDescriptor.cpp
		CDescriptor.hpp)

target_sources(${MAIN_PROJECT} PRIVATE ${WINDOWS_BLE_SRCS})