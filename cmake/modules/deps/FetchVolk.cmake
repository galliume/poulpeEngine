message(NOTICE "Fetching Volk from https://github.com/zeux/volk ...")

FetchContent_Declare(
  fetch_volk
  GIT_REPOSITORY https://github.com/zeux/volk
  GIT_TAG ${VOLK_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
)
FetchContent_MakeAvailable(fetch_volk)

target_include_directories(${PROJECT_NAME}
PRIVATE
  ${fetch_volk_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
PRIVATE
  volk)