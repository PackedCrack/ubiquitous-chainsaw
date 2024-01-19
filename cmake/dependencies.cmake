find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if(NOT CLANG_TIDY_EXE)
    message("clang-tidy not found")
endif()


macro(fetch_third_party)
    # --- fetch content --- #
    SET(FETCHCONTENT_BASE_DIR "${PROJECT_SOURCE_DIR}/_deps")
    SET(DEPENDENCY_BUILD_PATH "${CMAKE_BINARY_DIR}/dependencies/build")
    SET(DEPENDENCY_SRC_PATH "${CMAKE_BINARY_DIR}/dependencies/src")

    include(cmake/sdl.cmake)
    include(cmake/imgui.cmake)
endmacro()