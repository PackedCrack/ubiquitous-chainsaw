idf_component_register(
	SRCS 
	"CGapService.cpp"
	"CGapService.h"
	"CGattServer.cpp"
	"CGattServer.h"
	"CNimble.h"
	"CNimble.cpp"
	"CEventGroup.cpp" 
	"CEventLoop.cpp" 
	"CNonVolatileStorage.cpp" 
	"CChip.cpp" 
	"main.cpp"
	
	INCLUDE_DIRS 
	"${CMAKE_SOURCE_DIR}/../external/include"
	"${CMAKE_SOURCE_DIR}/../common"
	)
