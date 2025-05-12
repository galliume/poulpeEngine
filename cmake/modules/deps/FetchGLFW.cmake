set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_LIBRARY_TYPE SHARED CACHE STRING "" FORCE)

message(NOTICE "Fetching GLFW from https://github.com/glfw/glfw ...")
FetchContent_Declare(
  fetch_glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG ${GLFW_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
)
FetchContent_MakeAvailable(fetch_glfw)

target_include_directories(${PROJECT_NAME} PRIVATE ${fetch_glfw_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)