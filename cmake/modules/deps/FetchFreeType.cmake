message(NOTICE "Fetching FreeType from https://gitlab.freedesktop.org/freetype/freetype ...")

FetchContent_Declare(
  fetch_freetype
  GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype
  GIT_TAG ${FREETYPE_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
  SYSTEM
)
FetchContent_MakeAvailable(fetch_freetype)