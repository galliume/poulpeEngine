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

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
include(gitSubModule)

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
  list(APPEND DEBUG_OPTIONS /MDd /Od /Ob0 /RTC1 /Zi /Zc:__cplusplus /EHsc)
  list(APPEND RELEASE_OPTIONS /Ox /MD /Zc:__cplusplus /EHsc)

  #target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:/LTCG>)
  target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:/GA /Wall -Wno-c++98-compat -Wno-old-style-cast>)
else()
  list(APPEND DEBUG_OPTIONS -O0 -std=c++20 -Wall)
  list(APPEND RELEASE_OPTIONS -O3 -std=c++20 -Wall)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:PLP_DEBUG_BUILD>)

#########################################################

#########################################################
##################### External libs #####################
#########################################################
if (DEFINED ENV{VULKAN_SDK})
  cmake_path(SET VULKAN_PATH "$ENV{VULKAN_SDK}/Include")
  message(STATUS "Found Vulkan include dir: ${VULKAN_PATH}")
  find_package(Vulkan REQUIRED)
elseif (DEFINED ENV{VULKAN_SDK_ACTIONS})
  cmake_path(SET VULKAN_PATH "$ENV{VULKAN_SDK_ACTIONS}")
  message(STATUS "Found Vulkan via GitHub Actions include dir: ${VULKAN_PATH}")
  set(LOAD_VULKAN_LIB false)
else()
  message(FATAL_ERROR "The env var VULKAN_SDK nor VULKAN_SDK_ACTIONS not found.")
endif()

cmake_path(SET KTX_PATH "")

if (DEFINED ENV{KTX_SDK})
  cmake_path(SET KTX_PATH "$ENV{KTX_SDK}/include")
  message(STATUS "Found KTX include dir: ${KTX_PATH}")
  
  find_package(KTX REQUIRED)
endif()
  
target_include_directories(${PROJECT_NAME} PRIVATE
          ${PROJECT_SOURCE_DIR}/vendor/assimp/include
          ${PROJECT_SOURCE_DIR}/vendor/glm
          ${PROJECT_SOURCE_DIR}/vendor/KTX-Software/include
          ${PROJECT_SOURCE_DIR}/vendor/lua
          ${PROJECT_SOURCE_DIR}/vendor/miniaudio
          ${PROJECT_SOURCE_DIR}/vendor/nlohmann/include
          ${PROJECT_SOURCE_DIR}/vendor/spdlog/include
          ${PROJECT_SOURCE_DIR}/vendor/stb
          ${VULKAN_PATH}
          ${KTX_PATH})

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/assimp)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/nlohmann)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/volk)

#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#target_compile_options(assimp_viewer PRIVATE -w -Wno-error)
#endif()

#@todo include only used .c
add_library(
  lua STATIC
  ${PROJECT_SOURCE_DIR}/vendor/lua/lapi.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lauxlib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lbaselib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lcode.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lcorolib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lctype.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/ldblib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/ldebug.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/ldo.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/ldump.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lfunc.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lgc.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/linit.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/liolib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/llex.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lmathlib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lmem.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/loadlib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lobject.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lopcodes.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/loslib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lparser.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lstate.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lstring.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lstrlib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/ltable.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/ltablib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/ltests.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/ltm.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lundump.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lutf8lib.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lvm.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lzio.c
  ${PROJECT_SOURCE_DIR}/vendor/lua/lauxlib.h
  ${PROJECT_SOURCE_DIR}/vendor/lua/lua.h
  ${PROJECT_SOURCE_DIR}/vendor/lua/luaconf.h
  ${PROJECT_SOURCE_DIR}/vendor/lua/lualib.h
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  list(APPEND EXTRA_LIBS Ws2_32 user32)
else()
  #@todo find a way to install KTX on github server
  add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/KTX-Software)
endif()

list(APPEND EXTRA_LIBS glfw)
list(APPEND EXTRA_LIBS glm::glm)
list(APPEND EXTRA_LIBS lua)
list(APPEND EXTRA_LIBS nlohmann_json)
list(APPEND EXTRA_LIBS volk)
list(APPEND EXTRA_LIBS assimp::assimp)
list(APPEND EXTRA_LIBS spdlog::spdlog)
list(APPEND EXTRA_LIBS KTX::ktx)

target_link_libraries(${PROJECT_NAME} PRIVATE ${EXTRA_LIBS} poulpe)

list(APPEND LIB_DEBUG_OPTIONS ${DEBUG_OPTIONS})
list(APPEND LIB_RELEASE_OPTIONS ${RELEASE_OPTIONS})

target_compile_options(spdlog PRIVATE $<$<CONFIG:DEBUG>:${LIB_DEBUG_OPTIONS}>)
target_compile_options(spdlog PRIVATE $<$<CONFIG:RELEASE>:${LIB_RELEASE_OPTIONS}>)

target_compile_options(assimp PRIVATE $<$<CONFIG:DEBUG>:${LIB_DEBUG_OPTIONS}>)
target_compile_options(assimp PRIVATE $<$<CONFIG:RELEASE>:${LIB_RELEASE_OPTIONS}>)

set(PLP_LIB_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib")
set(PLP_BIN_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

set_target_properties(assimp PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
)

set_target_properties(glfw PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
)
set_target_properties(lua PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
)
set_target_properties(spdlog PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
)
set_target_properties(volk PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  RUNTIME_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
  ARCHIVE_OUTPUT_DIRECTORY ${PLP_BIN_DIR}
)
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
