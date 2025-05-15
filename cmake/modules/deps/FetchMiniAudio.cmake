message(NOTICE "Fetching MiniAudio from https://github.com/mackron/miniaudio ...")

FetchContent_Declare(
  fetch_mini_audio
  GIT_REPOSITORY https://github.com/mackron/miniaudio
  GIT_TAG ${MINIAUDIO_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
)
FetchContent_MakeAvailable(fetch_mini_audio)

target_include_directories(${PROJECT_NAME}
PRIVATE
  ${fetch_mini_audio_SOURCE_DIR})