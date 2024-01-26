include_directories("windows")

SET(BLUETOOTH_SRCS
        common.hpp
        CBLEScanner.hpp
)

target_sources(${MAIN_PROJECT} PRIVATE ${BLUETOOTH_SRCS})