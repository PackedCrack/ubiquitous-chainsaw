idf_component_register(
	SRCS 
	"main.cpp"

	"sys/CEventGroup.cpp"
	"sys/CEventLoop.cpp" 
	"sys/CNonVolatileStorage.cpp" 
	"sys/CChip.cpp" 
	"sys/CRestarter.cpp"

	"ble/Nimble.cpp"
	"ble/CGap.cpp"
	"ble/CGatt.cpp"
	"ble/CGattCharacteristic.hpp"
	"ble/CGattService.cpp"
	"ble/CAuthentication.cpp"
	
	
	
	INCLUDE_DIRS 
	"${CMAKE_SOURCE_DIR}/../external/include"
	"${CMAKE_SOURCE_DIR}/../common"
	)
