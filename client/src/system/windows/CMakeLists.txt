SET(WINDOWS_SRCS
        win32.hpp
        System.cpp
        CSystem.cpp
		CSystem.hpp
		CErrorMessage.cpp
		CErrorMessage.hpp
        CDeviceInfoSet.cpp
		CDeviceInfoSet.hpp
        CSerialCommunication.cpp
        CSerialCommunication.hpp
		CTrayIcon.cpp
		CTrayIcon.hpp)

target_sources(${MAIN_PROJECT} PRIVATE ${WINDOWS_SRCS})