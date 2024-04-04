SET(WINDOWS_SRCS
        CSystem.cpp
        CErrorMessage.cpp
        CTrayIcon.cpp)

target_sources(${MAIN_PROJECT} PRIVATE ${WINDOWS_SRCS})