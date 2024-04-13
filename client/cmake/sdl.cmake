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

FetchContent_GetProperties(${FETCH_DEP_SDL})
FetchContent_MakeAvailable(${FETCH_DEP_SDL})


set_target_properties(${FETCH_DEP_SDL} PROPERTIES CXX_STANDARD 23)
set_target_properties(${FETCH_DEP_SDL} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${LIB_OUTPUT_DIR}")

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

set(SDL_SHARED_DEFAULT OFF)
set(SDL_STATIC_DEFAULT ON)


# copy .so
add_custom_command(TARGET ${MAIN_PROJECT} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${FETCH_DEP_SDL}>
        $<TARGET_FILE_DIR:${MAIN_PROJECT}>)


# copy .a
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    SET(SDL_LIB_NAME "SDL3.lib")
else()
    SET(SDL_LIB_NAME "wolfssl.a")
endif()

# Copy the .a files to the LIB_OUTPUT_DIR directory - doing this manually because updating runtime output dir doesnt seem to work..
add_custom_command(OUTPUT "${LIB_OUTPUT_DIR}/${SDL_LIB_NAME}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL_BUILD_PATH}/${SDL_LIB_NAME}"
        "${LIB_OUTPUT_DIR}/${SDL_LIB_NAME}"
        DEPENDS "${SDL_BUILD_PATH}/${SDL_LIB_NAME}")


add_custom_target(COPY_SDL_STATIC_LIB ALL DEPENDS "${LIB_OUTPUT_DIR}/${SDL_LIB_NAME}")

add_dependencies(${MAIN_PROJECT} COPY_SDL_STATIC_LIB)