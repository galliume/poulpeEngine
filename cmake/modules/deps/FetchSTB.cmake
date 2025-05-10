message(NOTICE "Fetching STB from https://github.com/nothings/stb ...")
FetchContent_Declare(
  fetch_stb
  GIT_REPOSITORY https://github.com/nothings/stb
  GIT_TAG ${STB_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
)
FetchContent_MakeAvailable(fetch_stb)

target_include_directories(${PROJECT_NAME} PRIVATE ${fetch_stb_SOURCE_DIR})
