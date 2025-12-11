message(STATUS "Setting compiler definitions.")

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    VK_NO_PROTOTYPES
    GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_ENABLE_EXPERIMENTAL)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(${PROJECT_NAME} PRIVATE
      VOLK_STATIC_DEFINES
      _CRT_SECURE_NO_WARNINGS
      WIN32_LEAN_AND_MEAN)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
  $<$<CONFIG:DEBUG>:PLP_DEBUG_BUILD>)

message(STATUS "compiler definitions OK")