idf_component_register(
	SRCS 
	"main.cpp"
    "usb_cdc.cpp"
	"sys/CNonVolatileStorage.cpp" 
	"sys/CChip.cpp" 
	"sys/CSystem.cpp"
    
	"ble/CNimble.cpp"
	"ble/CGap.cpp"
	"ble/CConnection.cpp"
    "ble/ble_common.cpp"
	
	"ble/profiles/CProfileCache.cpp"
	"ble/profiles/CProfileCacheBuilder.cpp"
	"ble/profiles/CGattCharacteristic.hpp"
	"ble/profiles/CGattService.cpp"
	"ble/profiles/CWhoAmI.cpp"
	"ble/profiles/CWhereAmI.cpp"

	"${CMAKE_SOURCE_DIR}/../shared/security/CWolfCrypt.cpp"
	"${CMAKE_SOURCE_DIR}/../shared/security/CRandom.cpp"
	"${CMAKE_SOURCE_DIR}/../shared/security/ecc_key.cpp"
	

	
	INCLUDE_DIRS 
	"${CMAKE_SOURCE_DIR}/../external/include"
	"${CMAKE_SOURCE_DIR}/../shared"
	"include" # https://www.wolfssl.com/how-do-i-manage-the-build-configuration-of-wolfssl/
	)

set_target_properties(${COMPONENT_LIB} PROPERTIES CXX_STANDARD 23)


SET(warnings
        -Wall
        -Wextra
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wimplicit-fallthrough
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
)
target_compile_options(${COMPONENT_LIB} PRIVATE ${warnings})

include(../../cmake/pch.cmake)
use_pch(${COMPONENT_LIB})

include(../cmake/cppcheck.cmake)
add_dependencies(${COMPONENT_LIB} CPPCHECK_${COMPONENT_LIB})


file(GLOB_RECURSE ALL_SOURCE_FILES 
	"${CMAKE_SOURCE_DIR}/main/*.cpp"
	"${CMAKE_SOURCE_DIR}/main/*.hpp")
add_custom_target(
    clang-format
    COMMAND clang-format -i -style=file ${ALL_SOURCE_FILES}
    COMMENT "Running clang-format on all source files."
)
add_dependencies(${COMPONENT_LIB} clang-format)