idf_component_register(
	SRCS 
	"main.cpp"
	"sys/CNonVolatileStorage.cpp" 
	"sys/CChip.cpp" 
	"sys/CSystem.cpp"

	"ble/CNimble.cpp"
	"ble/CGap.cpp"
	"ble/CConnection.cpp"
	
	"ble/profiles/CProfileCache.cpp"
	"ble/profiles/CProfileCacheBuilder.cpp"
	"ble/profiles/CGattCharacteristic.hpp"
	"ble/profiles/CGattService.cpp"
	"ble/profiles/CWhoAmI.cpp"
	
	"${CMAKE_SOURCE_DIR}/../shared/security/CWolfCrypt.cpp"
	"${CMAKE_SOURCE_DIR}/../shared/security/sha.cpp"
	"${CMAKE_SOURCE_DIR}/../shared/security/CRandom.cpp"
	"${CMAKE_SOURCE_DIR}/../shared/security/ecc_key.cpp"


	
	INCLUDE_DIRS 
	"${CMAKE_SOURCE_DIR}/../external/include"
	"${CMAKE_SOURCE_DIR}/../shared"
	)


include(../../cmake/CompilerWarnings.cmake)
apply_compiler_warnings(${COMPONENT_LIB})

include(../../cmake/pch.cmake)
use_pch(${COMPONENT_LIB})

include(../cmake/cppcheck.cmake)
add_dependencies(${COMPONENT_LIB} CPPCHECK_${COMPONENT_LIB})
