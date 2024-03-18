idf_component_register(
	SRCS 
	"main.cpp"

	"sys/CEventGroup.cpp"
	"sys/CEventLoop.cpp" 
	"sys/CNonVolatileStorage.cpp" 
	"sys/CChip.cpp" 
	"sys/CSystem.cpp"

	"ble/CNimble.cpp"
	"ble/CGap.cpp"
	"ble/CGatt.cpp"
	
	"ble/profiles/CProfileCache.cpp"
	"ble/profiles/CProfileCacheBuilder.cpp"
	"ble/profiles/CGattCharacteristic.hpp"
	"ble/profiles/CGattService.cpp"
	"ble/profiles/CWhoAmI.cpp"
	
	
	
	INCLUDE_DIRS 
	"${CMAKE_SOURCE_DIR}/../external/include"
	"${CMAKE_SOURCE_DIR}/../common"
	)
