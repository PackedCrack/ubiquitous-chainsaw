if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    SET(DEFENSE_SRC
            defense_mechanism_win.cpp)
else()
    message(FATAL_ERROR "Only windows is implemented as of now")
endif()

target_sources(${MAIN_PROJECT} PRIVATE ${DEFENSE_SRC})