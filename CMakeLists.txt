#########################################################
##################### PoulpeEngine Project ##############
#########################################################
cmake_minimum_required(VERSION 3.28.0)

if (USE_CCACHE)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  endif()
else()
  message(WARNING "ccache deactivated")
endif()

project(PoulpeEngine
  VERSION 0.1.1
  DESCRIPTION "Game Engine with a vulkan renderer"
  HOMEPAGE_URL "https://github.com/galliume/poulpeEngine"
  LANGUAGES C CXX
)

add_library(poulpe INTERFACE)

#target_compile_features(poulpe INTERFACE cxx_std_20)
message(STATUS "Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Build type" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

add_executable(${PROJECT_NAME} src/Poulpe.cpp)

configure_file(PoulpeEngineConfig.h.in PoulpeEngineConfig.h)

add_definitions(-DVK_NO_PROTOTYPES
                -DVOLK_STATIC_DEFINES
                -D_CRT_SECURE_NO_WARNINGS
                -DDWIN32_LEAN_AND_MEAN)

target_precompile_headers(${PROJECT_NAME} PRIVATE include/Poulpepch.hpp.pch)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Component)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Component/Renderer)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Core)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Core/Network)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/GUI)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Manager)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Renderer)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Renderer/Vulkan)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Utils)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}")

set(DEBUG_OPTIONS -g)
set(RELEASE_OPTIONS )

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")

  set(USE_CCACHE OFF CACHE BOOL "" FORCE)
  set(CMAKE_CXX_COMPILER_LAUNCHER "")
  set(CMAKE_C_COMPILER_LAUNCHER "")

  list(APPEND DEBUG_OPTIONS /MDd /Od /Ob0 /RTC1 /Zi /Zc:__cplusplus /EHsc)
  list(APPEND RELEASE_OPTIONS /Ox /MD /Zc:__cplusplus /EHsc)

  #target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:/LTCG>)
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:/GA /Wall -Wno-c++98-compat -Wno-old-style-cast>)
else()
    list(APPEND DEBUG_OPTIONS -O0 -Wall)
    list(APPEND RELEASE_OPTIONS -O3 -Wall)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:PLP_DEBUG_BUILD>)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  list(APPEND EXTRA_LIBS Ws2_32 user32)
else()
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)
  list(APPEND EXTRA_LIBS Threads::Threads)
endif()

#########################################################

#########################################################
##################### External libs #####################
#########################################################
find_package(Vulkan REQUIRED)
list(APPEND EXTRA_LIBS Vulkan::Vulkan)

set(PLP_LIB_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib")
set(PLP_BIN_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

set(FETCHCONTENT_QUIET OFF)
include(fetchDeps)

target_link_libraries(${PROJECT_NAME} PRIVATE ${EXTRA_LIBS} poulpe)

list(APPEND LIB_DEBUG_OPTIONS ${DEBUG_OPTIONS})
list(APPEND LIB_RELEASE_OPTIONS ${RELEASE_OPTIONS})

#########################################################

#########################################################
##################### Miscellaneous #####################
#########################################################
#Filters for visual studio structure
file(GLOB_RECURSE _source_list "${PROJECT_SOURCE_DIR}/src/**/*.cpp" "${PROJECT_SOURCE_DIR}/src/**/*.hpp")
file(GLOB_RECURSE _source_list "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.hpp")
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
  list(APPEND TIDY_SRC "${_source}")
endforeach()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL$<$<CONFIG:Debug>:Debug>")

set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

add_custom_target(
        CompilShaders ${CMAKE_CURRENT_SOURCE_DIR}/bin/shadersCompilation.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

#add_custom_target(graphviz ALL
#    COMMAND ${CMAKE_COMMAND} "--graphviz=graph/plp.dot" ..
#    COMMAND dot -Tsvg graph/plp.dot -o graph/plp.svg
#    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
#)
#########################################################
