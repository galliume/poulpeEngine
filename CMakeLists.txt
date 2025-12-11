##################### PoulpeEngine Project ##############

cmake_minimum_required(VERSION 3.31.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Build type" FORCE)

include(cmake_checks)

project(PoulpeEngine
  VERSION 0.2.1
  DESCRIPTION "Game Engine with a vulkan renderer"
  HOMEPAGE_URL "https://github.com/galliume/poulpeEngine"
  LANGUAGES C CXX
  )

add_executable(${PROJECT_NAME} src/Poulpe.cpp poulpeEngine.rc)

include(programs)

message(STATUS "Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

configure_file(PoulpeEngineConfig.h.in PoulpeEngineConfig.h)

include(packages)
include(subdirs)
include(compiler_definitions)
include(compiler_flags)
include(compiler_links)
include(fetchDeps)
include(install)

include(custom_targets)
include(graphVizOptions)

#include(CheckIPOSupported)
#check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
#if(ipo_supported)
#  message(STATUS "ipo_supported")
#  set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
#endif()
