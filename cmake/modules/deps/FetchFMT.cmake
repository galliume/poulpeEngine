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
)
FetchContent_MakeAvailable(fetch_fmt)

if(TARGET fmt)
  target_compile_options(fmt PRIVATE
    $<$<C_COMPILER_ID:Clang>:-Wno-nan-infinity-disabled>
    $<$<C_COMPILER_ID:AppleClang>:-Wno-nan-infinity-disabled>
    $<$<C_COMPILER_ID:GNU>:-Wno-nan-infinity-disabled>
  )
endif()

target_include_directories(${PROJECT_NAME}
SYSTEM PRIVATE
  ${fetch_fmt_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
PRIVATE
  fmt::fmt)