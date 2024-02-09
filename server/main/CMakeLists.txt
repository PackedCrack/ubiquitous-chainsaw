idf_component_register(
	SRCS 
	"CChainsaw.cpp"
	"CGapService.cpp"
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
