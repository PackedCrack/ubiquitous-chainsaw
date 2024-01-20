SET(MAIN_SRCS 
    main.cpp
)

add_subdirectory("gfx")
add_subdirectory("gui")
add_subdirectory("common")
add_subdirectory("bluetooth")

target_sources(${MAIN_PROJECT} PRIVATE ${MAIN_SRCS})