SET(BLUETOOTH_SRCS
        CScanner.cpp
        CScanner.hpp
)

target_sources(${MAIN_PROJECT} PRIVATE ${BLUETOOTH_SRCS})