SET(SYSTEM_SRCS
        System.cpp
        CErrorMessage.cpp
        CTrayIcon.cpp)

target_sources(${MAIN_PROJECT} PRIVATE ${SYSTEM_SRCS})