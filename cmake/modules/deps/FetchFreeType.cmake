message(NOTICE "Fetching FreeType from https://gitlab.freedesktop.org/freetype/freetype ...")
FetchContent_Declare(
  fetch_freetype
  GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype
  GIT_TAG  VER-2-13-3
  GIT_SHALLOW FALSE
  GIT_PROGRESS TRUE
  UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(fetch_freetype)

target_include_directories(${PROJECT_NAME} PRIVATE ${fetch_freetype_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE freetype)

set_target_properties(freetype PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
)