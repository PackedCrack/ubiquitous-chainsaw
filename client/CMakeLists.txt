cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0076 NEW)


SET(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(LIB_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lib")


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
SET(FOLDER_MAIN "chainsaw")
SET(FOLDER_TEST "test")


SET(MAIN_PROJECT chainsaw)
SET(TEST_PROJECT TESTS_${MAIN_PROJECT})
project(${MAIN_PROJECT} LANGUAGES C CXX)
add_executable(${MAIN_PROJECT}
        "client.rc")
#add_dependencies(${MAIN_PROJECT} CPPCHECK_${MAIN_PROJECT})
#add_dependencies(${MAIN_PROJECT} CHILD_HEADER_FINDER)


include(FetchContent)
include(cmake/dependencies.cmake)
include(../cmake/CompilerWarnings.cmake)
include(../cmake/pch.cmake)
include(cmake/cppcheck.cmake)
include(cmake/child_header_finder.cmake)

fetch_third_party()
add_subdirectory("../shared/security" "${CMAKE_BINARY_DIR}/security")

add_subdirectory("src")
# properties
set_target_properties(${MAIN_PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
set_target_properties(${SECURITY_PROJECT} PROPERTIES OUTPUT_NAME_DEBUG "${MAIN_PROJECT}d")
set_target_properties(${SECURITY_PROJECT} PROPERTIES OUTPUT_NAME_RELEASE "${MAIN_PROJECT}")
set_target_properties(${MAIN_PROJECT} PROPERTIES CXX_STANDARD 23)
set_target_properties(${MAIN_PROJECT} PROPERTIES C_STANDARD 11)
set_target_properties(${MAIN_PROJECT} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${MAIN_PROJECT} PROPERTIES C_STANDARD_REQUIRED ON)
set_target_properties(${MAIN_PROJECT} PROPERTIES FOLDER ${FOLDER_MAIN})

#include
target_include_directories(${MAIN_PROJECT} PRIVATE "${CMAKE_SOURCE_DIR}/../external/include")
target_include_directories(${MAIN_PROJECT} PRIVATE "${CMAKE_SOURCE_DIR}/../shared")
target_include_directories(${MAIN_PROJECT} PRIVATE "${SDL_SRC_PATH}/include")
target_include_directories(${MAIN_PROJECT} PRIVATE "${IMGUI_SRC_PATH}/include")
target_include_directories(${MAIN_PROJECT} PRIVATE "${WOLFSSL_SRC_PATH}/wolfssl")

#link
target_link_directories(${MAIN_PROJECT} PRIVATE ${LIB_OUTPUT_DIR})
target_link_libraries(${MAIN_PROJECT} PRIVATE ${FETCH_DEP_SDL})
target_link_libraries(${MAIN_PROJECT} PRIVATE ${FETCH_DEP_IMGUI})
target_link_libraries(${MAIN_PROJECT} PRIVATE ${FETCH_DEP_WOLFSSL})

# compiler settings etc
apply_compiler_warnings(${MAIN_PROJECT})
use_pch(${MAIN_PROJECT})

get_target_property(PROJECT_SOURCES ${MAIN_PROJECT} SOURCES)
source_group(TREE "${PROJECT_SOURCE_DIR}/.." FILES ${PROJECT_SOURCES})


# testing
enable_testing()
add_subdirectory("test")
set_target_properties(${TEST_PROJECT} PROPERTIES FOLDER ${FOLDER_TEST})
set_target_properties(gtest PROPERTIES FOLDER ${FOLDER_TEST})
set_target_properties(gtest_main PROPERTIES FOLDER ${FOLDER_TEST})
# include dirs
target_include_directories(${TEST_PROJECT} PRIVATE "${PROJECT_SOURCE_DIR}/external/include")
target_include_directories(${TEST_PROJECT} PRIVATE "${PROJECT_SOURCE_DIR}/src")
# Hack because some includes uses the "wolfssl directory explicitly and others dont"
target_include_directories(${TEST_PROJECT} PRIVATE "${WOLFSSL_SRC_PATH}")
target_include_directories(${TEST_PROJECT} PRIVATE "${WOLFSSL_SRC_PATH}/wolfssl")

target_link_libraries(${TEST_PROJECT} PRIVATE ${FETCH_DEP_WOLFSSL})


