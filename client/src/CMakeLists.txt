SET(MAIN_SRCS 
        main.cpp
        resource.hpp
        win32.hpp
        client_defines.hpp
)

add_subdirectory("system")
add_subdirectory("gfx")
add_subdirectory("gui")
add_subdirectory("common")
add_subdirectory("bluetoothLE")

target_sources(${MAIN_PROJECT} PRIVATE ${MAIN_SRCS})