SET(MAIN_SRCS 
        main.cpp
		CServer.cpp
		CServer.hpp
		CRssiDemander.cpp
		CRssiDemander.hpp
		CReplayProtector.cpp
		CReplayProtector.hpp
		client_common.cpp
		client_common.hpp
        resource.hpp
        client_defines.hpp
)

add_subdirectory("system")
add_subdirectory("gfx")
add_subdirectory("gui")
add_subdirectory("common")
add_subdirectory("bluetoothLE")

target_sources(${MAIN_PROJECT} PRIVATE ${MAIN_SRCS})