message(NOTICE "Fetching Volk from https://github.com/zeux/volk ...")

FetchContent_Declare(
  fetch_volk
  GIT_REPOSITORY https://github.com/zeux/volk
  GIT_TAG ${VOLK_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
  SYSTEM)

FetchContent_MakeAvailable(fetch_volk)