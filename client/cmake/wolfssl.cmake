SET(FETCH_DEP_WOLFSSL wolfssl)
SET(WOLFSSL_SRC_PATH "${DEPENDENCY_SRC_PATH}/wolfssl-src")
SET(WOLFSSL_BUILD_PATH "${DEPENDENCY_BUILD_PATH}/wolfssl-build")


FetchContent_Declare(
  ${FETCH_DEP_WOLFSSL}
  GIT_REPOSITORY    https://github.com/wolfSSL/wolfssl
  GIT_TAG           master
  SOURCE_DIR        "${WOLFSSL_SRC_PATH}"
  BINARY_DIR        "${WOLFSSL_BUILD_PATH}"
)

# configuration settings..
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "Only build as a static lib")
SET(WOLFSSL_OLD_NAMES "no" CACHE STRING "Define to not use old wolfssl names")

SET(WOLFSSL_ECC "yes" CACHE STRING "Include ECC algorithms")
SET(WOLFSSL_AESSIV "yes" CACHE STRING "Include AESSIV algorithms")
SET(WOLFSSL_AESCTR "yes" CACHE STRING "Include AESCTR algorithms")
SET(WOLFSSL_AESCCM "yes" CACHE STRING "Include AESCCM algorithms")
SET(WOLFSSL_AESOFB "yes" CACHE STRING "Include AESOFB algorithms")
SET(WOLFSSL_AESCFB "yes" CACHE STRING "Include AESCFB algorithms")
SET(WOLFSSL_HKDF "yes" CACHE STRING "Include HKDF algorithms")
SET(WOLFSSL_ECCSHAMIR "yes" CACHE STRING "Include ECCSHAMIR algorithms")
SET(WOLFSSL_CURVE25519 "yes" CACHE STRING "Include CURVE25519 algorithms")
SET(WOLFSSL_ED25519 "yes" CACHE STRING "Include ED25519 algorithms")
SET(WOLFSSL_CURVE448 "yes" CACHE STRING "Include CURVE448 algorithms")
SET(WOLFSSL_X963KDF "yes" CACHE STRING "Include X963KDF algorithms")
SET(WOLFSSL_HPKE "yes" CACHE STRING "Include HPKE algorithms")



FetchContent_GetProperties(${FETCH_DEP_WOLFSSL})
FetchContent_MakeAvailable(${FETCH_DEP_WOLFSSL})

# this is a hack because wolfssl's test code fails to compile because for some reason does not include stdint.h (?)
target_precompile_headers(unit_test PRIVATE <stdint.h>)


set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES CXX_STANDARD 23)
set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")

set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${RUNTIME_OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${RUNTIME_OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE "${RUNTIME_OUTPUT_DIRECTORY}")

set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${LIB_OUTPUT_DIR}")
set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${LIB_OUTPUT_DIR}")
set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${LIB_OUTPUT_DIR}")

set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES OUTPUT_NAME_DEBUG "${FETCH_DEP_WOLFSSL}d")
set_target_properties(${FETCH_DEP_WOLFSSL} PROPERTIES OUTPUT_NAME_RELEASE "${FETCH_DEP_WOLFSSL}")


SET(FOLDER_WOLFSSL "wolfssl")
if(use_folders_set)
	# not 100% sure about the names for these targets - they are inside wolfssl's CMakeLists.txt files (somewhere)
	set_target_properties(client PROPERTIES FOLDER ${FOLDER_WOLFSSL})
	set_target_properties(echoclient PROPERTIES FOLDER ${FOLDER_WOLFSSL})
	set_target_properties(echoserver PROPERTIES FOLDER ${FOLDER_WOLFSSL})
	set_target_properties(server PROPERTIES FOLDER ${FOLDER_WOLFSSL})
	set_target_properties(unit_test PROPERTIES FOLDER ${FOLDER_WOLFSSL})
	set_target_properties(wolfcryptbench PROPERTIES FOLDER ${FOLDER_WOLFSSL})
	set_target_properties(wolfcrypttest PROPERTIES FOLDER ${FOLDER_WOLFSSL})
	set_target_properties(wolfssl PROPERTIES FOLDER ${FOLDER_WOLFSSL})
endif()