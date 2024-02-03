cmake_minimum_required(VERSION 3.20)


SET(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
SET(LIB_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lib/")


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
SET(FOLDER_MAIN "chainsaw")
SET(FOLDER_TEST "test")

SET(MAIN_PROJECT chainsaw)
SET(TEST_PROJECT TESTS)
project(${MAIN_PROJECT} LANGUAGES C CXX)


include(FetchContent)
include(cmake/dependencies.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/pch.cmake)
include(cmake/cppcheck.cmake)

fetch_third_party()



# Initialize third-party projects
#add_subdirectory("third-party")


add_executable(${MAIN_PROJECT}
        src/bluetoothLE/CBLEScanner.hpp
        src/bluetoothLE/windows/CScanner.hpp
        src/bluetoothLE/windows/CScanner.cpp
        src/bluetoothLE/common.hpp
        src/common/CThreadSafeHashMap.hpp)
#add_dependencies(${MAIN_PROJECT} CPPCHECK)
add_subdirectory("src")
# properties
set_target_properties(${MAIN_PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
set_target_properties(${MAIN_PROJECT} PROPERTIES CXX_STANDARD 23)
set_target_properties(${MAIN_PROJECT} PROPERTIES C_STANDARD 11)
set_target_properties(${MAIN_PROJECT} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${MAIN_PROJECT} PROPERTIES C_STANDARD_REQUIRED ON)
set_target_properties(${MAIN_PROJECT} PROPERTIES FOLDER ${FOLDER_MAIN})
#include
target_include_directories(${MAIN_PROJECT} PRIVATE "${SDL_SRC_PATH}/include")
target_include_directories(${MAIN_PROJECT} PRIVATE "${IMGUI_SRC_PATH}/include")
#link
target_link_directories(${MAIN_PROJECT} PRIVATE ${LIB_OUTPUT_DIR})
target_link_libraries(${MAIN_PROJECT} PRIVATE ${FETCH_DEP_SDL})
target_link_libraries(${MAIN_PROJECT} PRIVATE ${FETCH_DEP_IMGUI})
#target_link_libraries(${MAIN_PROJECT} PRIVATE libbluetooth.so)



# compiler settings etc
apply_compiler_warnings(${MAIN_PROJECT})
use_pch(${MAIN_PROJECT})


enable_testing()
add_subdirectory("test")
set_target_properties(${TEST_PROJECT} PROPERTIES FOLDER ${FOLDER_TEST})
set_target_properties(gtest PROPERTIES FOLDER ${FOLDER_TEST})
set_target_properties(gtest_main PROPERTIES FOLDER ${FOLDER_TEST})
# include dirs
target_include_directories(${TEST_PROJECT} PRIVATE "${PROJECT_SOURCE_DIR}/external/include")
target_include_directories(${TEST_PROJECT} PRIVATE "${PROJECT_SOURCE_DIR}/src")