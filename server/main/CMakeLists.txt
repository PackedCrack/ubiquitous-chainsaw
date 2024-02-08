idf_component_register(
	SRCS 
	"CGapService.cpp"
	"CGattServer.cpp"
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
