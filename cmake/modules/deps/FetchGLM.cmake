message(NOTICE "Fetching GLM from https://github.com/g-truc/glm ...")

set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
set(GLM_ENABLE_EXPERIMENTAL ON CACHE BOOL "" FORCE)

#need to detected if the compiler supports AVX2 or SSEx
set(GLM_ENABLE_SIMD_AVX2 ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  fetch_glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG ${GLM_TAG}
  GIT_SHALLOW ${FETCH_GIT_SHALLOW}
  GIT_PROGRESS ${FETCH_GIT_PROGRESS}
  UPDATE_DISCONNECTED ${FETCH_UPDATE_DISCONNECTED}
  SYSTEM)

FetchContent_MakeAvailable(fetch_glm)