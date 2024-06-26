SET(FETCH_DEP_IMGUI imgui)
SET(IMGUI_SRC_PATH "${DEPENDENCY_SRC_PATH}/${FETCH_DEP_IMGUI}-src")
SET(IMGUI_BUILD_PATH "${DEPENDENCY_BUILD_PATH}/${FETCH_DEP_IMGUI}-build")

FetchContent_Declare(
  ${FETCH_DEP_IMGUI}
  GIT_REPOSITORY    https://github.com/ocornut/imgui
  GIT_TAG           docking
  SOURCE_DIR        "${IMGUI_SRC_PATH}"
  BINARY_DIR        "${IMGUI_BUILD_PATH}"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

#FetchContent_MakeAvailable(${DEP_NAME})

FetchContent_GetProperties(${FETCH_DEP_IMGUI})
if(NOT ${FETCH_DEP_IMGUI}_POPULATED)
  FetchContent_Populate(${FETCH_DEP_IMGUI})
endif()

SET(IMGUI_SOURCES ${IMGUI_SRC_PATH}/imconfig.h
    ${IMGUI_SRC_PATH}/imgui_internal.h
    ${IMGUI_SRC_PATH}/imgui_tables.cpp
    ${IMGUI_SRC_PATH}/imstb_rectpack.h
    ${IMGUI_SRC_PATH}/imstb_textedit.h
    ${IMGUI_SRC_PATH}/imstb_truetype.h

    ${IMGUI_SRC_PATH}/imgui.h
    ${IMGUI_SRC_PATH}/imgui.cpp

    ${IMGUI_SRC_PATH}/imgui_demo.cpp
    ${IMGUI_SRC_PATH}/imgui_draw.cpp
    ${IMGUI_SRC_PATH}/imgui_widgets.cpp

    ${IMGUI_SRC_PATH}/backends/imgui_impl_sdl3.h
    ${IMGUI_SRC_PATH}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_SRC_PATH}/backends/imgui_impl_sdlrenderer3.h
    ${IMGUI_SRC_PATH}/backends/imgui_impl_sdlrenderer3.cpp
    )

add_library(${FETCH_DEP_IMGUI} STATIC)
set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES CXX_STANDARD 23)
set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")

set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${RUNTIME_OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${RUNTIME_OUTPUT_DIRECTORY}")
set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE "${RUNTIME_OUTPUT_DIRECTORY}")

set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${LIB_OUTPUT_DIR}")
set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${LIB_OUTPUT_DIR}")
set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${LIB_OUTPUT_DIR}")

set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES OUTPUT_NAME_DEBUG "${FETCH_DEP_IMGUI}d")
set_target_properties(${FETCH_DEP_IMGUI} PROPERTIES OUTPUT_NAME_RELEASE "${FETCH_DEP_IMGUI}")


target_sources(${FETCH_DEP_IMGUI} PRIVATE ${IMGUI_SOURCES})

target_include_directories(${FETCH_DEP_IMGUI} PRIVATE ${IMGUI_SRC_PATH})
target_include_directories(${FETCH_DEP_IMGUI} PRIVATE ${SDL_SRC_PATH}/include/)

target_link_directories(${FETCH_DEP_IMGUI} PRIVATE ${LIB_OUTPUT_DIR})
target_link_libraries(${FETCH_DEP_IMGUI} PRIVATE ${FETCH_DEP_SDL})

file(COPY ${IMGUI_SOURCES} DESTINATION ${IMGUI_SRC_PATH}/include/imgui/)
