SET(WINDOWS_SRCS
		CSystem.cpp
		CSystem.hpp
		CErrorMessage.cpp
		CErrorMessage.hpp
		CTrayIcon.cpp
		CTrayIcon.hpp)

target_sources(${MAIN_PROJECT} PRIVATE ${WINDOWS_SRCS})