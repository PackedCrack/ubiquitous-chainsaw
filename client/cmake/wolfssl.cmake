SET(FETCH_DEP_WOLFSSL wolfssl)
SET(WOLFSSL_SRC_PATH "${DEPENDENCY_SRC_PATH}/wolfssl-src")
SET(WOLFSSL_BUILD_PATH "${DEPENDENCY_BUILD_PATH}/wolfssl-build")

FetchContent_Declare(
  ${FETCH_DEP_WOLFSSL}
  GIT_REPOSITORY    https://github.com/wolfSSL/wolfssl
  GIT_TAG           master
  SOURCE_DIR        "${WOLFSSL_SRC_PATH}"
  BINARY_DIR        "${WOLFSSL_BUILD_PATH}"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  CMAKE_ARGS        -DBUILD_SHARED_LIBS=OFF
)

FetchContent_GetProperties(${FETCH_DEP_WOLFSSL})
FetchContent_MakeAvailable(${FETCH_DEP_WOLFSSL})


set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES CXX_STANDARD 23)
set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${LIB_OUTPUT_DIR}")


if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    SET(WOLFSSL_LIB_NAME "wolfssl.lib")
else()
    SET(WOLFSSL_LIB_NAME "wolfssl.a")
endif()

# Copy the .a files to the LIB_OUTPUT_DIR directory - doing this manually because updating runtime output dir doesnt seem to work..
add_custom_command(OUTPUT "${LIB_OUTPUT_DIR}/${WOLFSSL_LIB_NAME}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WOLFSSL_BUILD_PATH}/${WOLFSSL_LIB_NAME}"
        "${LIB_OUTPUT_DIR}/${WOLFSSL_LIB_NAME}"
        DEPENDS "${WOLFSSL_BUILD_PATH}/${WOLFSSL_LIB_NAME}")


add_custom_target(COPY_WOLF_STATIC_LIB ALL DEPENDS "${LIB_OUTPUT_DIR}/${WOLFSSL_LIB_NAME}")

add_dependencies(${MAIN_PROJECT} COPY_WOLF_STATIC_LIB)





# If you want to use the .dll - this will copy the .dll to the executable binary directory
#add_custom_command(TARGET ${MAIN_PROJECT} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#    $<TARGET_FILE:${FETCH_DEP_WOLFSSL}>
#    $<TARGET_FILE_DIR:${MAIN_PROJECT}>)