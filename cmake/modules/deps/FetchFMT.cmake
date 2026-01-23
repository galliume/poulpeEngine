message(NOTICE "Fetching FMT from https://github.com/fmtlib/fmt ...")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL" CACHE STRING "" FORCE)
  endif()
endif()

FetchContent_Declare(
  fetch_fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG ${FMT_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
  SYSTEM
)
FetchContent_MakeAvailable(fetch_fmt)

# # Mark fmt includes as SYSTEM to suppress warnings from third-party code
# get_target_property(FMT_INC_DIR fmt INTERFACE_INCLUDE_DIRECTORIES)
# if(FMT_INC_DIR)
#   set_target_properties(fmt PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${FMT_INC_DIR}")
# endif()