message(NOTICE "Fetching Nlohmann Json from https://github.com/nlohmann/json ...")

FetchContent_Declare(
  fetch_nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG ${NH_JSON_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
)
FetchContent_MakeAvailable(fetch_nlohmann_json)

target_include_directories(${PROJECT_NAME}
PRIVATE
  ${fetch_nlohmann_json_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
PRIVATE 
  nlohmann_json)