set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_DOC OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_JNI OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_PY OFF CACHE BOOL "" FORCE)
set(KTX_FEATURE_TOOLS_CTS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

#set(CMAKE_BINARY_DIR_RESET ${CMAKE_BINARY_DIR})
#set(CMAKE_BINARY_DIR ${fetch_ktx_BINARY_DIR} CACHE PATH "" FORCE)

message(NOTICE "Fetching LibKTX from https://github.com/KhronosGroup/KTX-Software ...")

FetchContent_Declare(
  fetch_ktx
  GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software
  GIT_TAG ${KTX_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
)
FetchContent_MakeAvailable(fetch_ktx)

target_include_directories(${PROJECT_NAME}
SYSTEM PRIVATE
  ${fetch_ktx_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
PRIVATE
  ktx)

#set(CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR_RESET} CACHE PATH "" FORCE)