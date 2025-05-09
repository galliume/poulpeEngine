message(NOTICE "Fetching GLM from https://github.com/g-truc/glm ...")
FetchContent_Declare(
  fetch_glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG  1.0.1
  GIT_SHALLOW FALSE
  GIT_PROGRESS TRUE
  UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(fetch_glm)

target_include_directories(${PROJECT_NAME} PRIVATE ${fetch_glm_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)