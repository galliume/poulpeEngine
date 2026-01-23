set(wxBUILD_SHARED ON CACHE BOOL "" FORCE)

message(NOTICE "Fetching WxWidget from https://github.com/wxWidgets/wxWidgets ...")
FetchContent_Declare(
  fetch_WxWidgets
  GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets
  GIT_TAG ${WxWidget_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
  SYSTEM
)
FetchContent_MakeAvailable(fetch_WxWidgets)