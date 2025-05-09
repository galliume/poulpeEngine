message(NOTICE "Fetching STB from https://github.com/nothings/stb ...")
FetchContent_Declare(
  fetch_stb
  GIT_REPOSITORY https://github.com/nothings/stb
  GIT_TAG  master
  GIT_SHALLOW FALSE
  GIT_PROGRESS TRUE
  UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(fetch_stb)

target_include_directories(${PROJECT_NAME} PRIVATE ${fetch_stb_SOURCE_DIR})