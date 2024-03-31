SET(MAIN_SRCS 
    main.cpp
)

add_subdirectory("system")
add_subdirectory("gfx")
add_subdirectory("gui")
add_subdirectory("common")
add_subdirectory("bluetoothLE")
add_subdirectory("defense")
add_subdirectory("security")

target_sources(${MAIN_PROJECT} PRIVATE ${MAIN_SRCS})