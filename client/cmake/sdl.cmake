SET(FETCH_DEP_SDL SDL3-shared)
SET(SDL_SRC_PATH "${DEPENDENCY_SRC_PATH}/sdl-src")
SET(SDL_BUILD_PATH "${DEPENDENCY_BUILD_PATH}/sdl-build")

FetchContent_Declare(
  ${FETCH_DEP_SDL}
  GIT_REPOSITORY    https://github.com/libsdl-org/SDL
  GIT_TAG           main
  SOURCE_DIR        "${SDL_SRC_PATH}"
  BINARY_DIR        "${SDL_BUILD_PATH}"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  CMAKE_ARGS        -DSDL_SHARED_DEFAULT=OFF -DSDL_STATIC_DEFAULT ON
)

SET(SDL_SHARED_DEFAULT OFF CACHE BOOL "Only build as a static lib")
SET(SDL_STATIC_DEFAULT ON CACHE BOOL "Only build as a static lib")

FetchContent_GetProperties(${FETCH_DEP_SDL})
FetchContent_MakeAvailable(${FETCH_DEP_SDL})


set_target_properties(${FETCH_DEP_SDL} PROPERTIES CXX_STANDARD 23)
set_target_properties(${FETCH_DEP_SDL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_SDL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_SDL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")

set_target_properties(${FETCH_DEP_SDL} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${RUNTIME_OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_SDL} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${RUNTIME_OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_SDL} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE "${RUNTIME_OUTPUT_DIRECTORY}")

set_target_properties(${FETCH_DEP_SDL} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${LIB_OUTPUT_DIR}")
set_target_properties(${FETCH_DEP_SDL} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${LIB_OUTPUT_DIR}")
set_target_properties(${FETCH_DEP_SDL} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${LIB_OUTPUT_DIR}")

set_target_properties(${FETCH_DEP_SDL} PROPERTIES OUTPUT_NAME_DEBUG "${FETCH_DEP_SDL}d")
set_target_properties(${FETCH_DEP_SDL} PROPERTIES OUTPUT_NAME_RELEASE "${FETCH_DEP_SDL}")


file(GLOB_RECURSE SDL_SOURCES
      ${SDL_SRC_PATH}/include/*.cpp
      ${SDL_SRC_PATH}/include/*.hpp
      ${SDL_SRC_PATH}/include/*.c
      ${SDL_SRC_PATH}/include/*.h
)
list(REMOVE_ITEM SDL_SOURCES 
${SDL_SRC_PATH}/include/SDL_config.h
${SDL_SRC_PATH}/include/SDL_config_android.h
${SDL_SRC_PATH}/include/SDL_config_emscripten.h
${SDL_SRC_PATH}/include/SDL_config_iphoneos.h
${SDL_SRC_PATH}/include/SDL_config_macosx.h
${SDL_SRC_PATH}/include/SDL_config_minimal.h
${SDL_SRC_PATH}/include/SDL_config_ngage.h
${SDL_SRC_PATH}/include/SDL_config_os2.h
${SDL_SRC_PATH}/include/SDL_config_pandora.h
${SDL_SRC_PATH}/include/SDL_config_windows.h
${SDL_SRC_PATH}/include/SDL_config_wingdk.h
${SDL_SRC_PATH}/include/SDL_config_winrt.h
${SDL_SRC_PATH}/include/SDL_config_xbox.h
)
file(COPY ${SDL_SOURCES} DESTINATION ${SDL_BUILD_PATH}/include/)
