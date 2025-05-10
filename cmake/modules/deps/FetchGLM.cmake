message(NOTICE "Fetching GLM from https://github.com/g-truc/glm ...")
FetchContent_Declare(
  fetch_glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG ${GLM_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
)
FetchContent_MakeAvailable(fetch_glm)

target_include_directories(${PROJECT_NAME} PRIVATE ${fetch_glm_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

set_target_properties(glm PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
)