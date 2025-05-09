message(NOTICE "Fetching MiniAudio from https://github.com/mackron/miniaudio ...")
FetchContent_Declare(
  fetch_mini_audio
  GIT_REPOSITORY https://github.com/mackron/miniaudio
  GIT_TAG  0.11.22
  GIT_SHALLOW FALSE
  GIT_PROGRESS TRUE
  UPDATE_DISCONNECTED TRUE
)
FetchContent_MakeAvailable(fetch_mini_audio)

target_include_directories(${PROJECT_NAME} PRIVATE ${fetch_mini_audio_SOURCE_DIR})