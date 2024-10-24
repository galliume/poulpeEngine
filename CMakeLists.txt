#########################################################
##################### PoulpeEngine Project ##############
#########################################################
cmake_minimum_required(VERSION 3.25.2)

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

target_compile_features(poulpe INTERFACE cxx_std_20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED            ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)
set(GIT_SUBMODULE ON)
set(GLFW_LIBRARY_TYPE STATIC)
#weird bug with vulkan loader ?
set(CMAKE_MSVC_RUNTIME_LIBRARY "")
#need to find how to have wayland on github actions...
set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_BUILD_X11 ON)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})

#assimp format
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_AMF_IMPORTER OFF)
set(ASSIMP_BUILD_3DS_IMPORTER OFF)
set(ASSIMP_BUILD_AC_IMPORTER OFF)
set(ASSIMP_BUILD_ASE_IMPORTER OFF)
set(ASSIMP_BUILD_ASSBIN_IMPORTER OFF)
set(ASSIMP_BUILD_B3D_IMPORTER OFF)
set(ASSIMP_BUILD_BVH_IMPORTER OFF)
set(ASSIMP_BUILD_COLLADA_IMPORTER OFF)
set(ASSIMP_BUILD_DXF_IMPORTER OFF)
set(ASSIMP_BUILD_CSM_IMPORTER OFF)
set(ASSIMP_BUILD_HMP_IMPORTER OFF)
set(ASSIMP_BUILD_IRRMESH_IMPORTER OFF)
set(ASSIMP_BUILD_IQM_IMPORTER OFF)
set(ASSIMP_BUILD_IRR_IMPORTER OFF)
set(ASSIMP_BUILD_LWO_IMPORTER OFF)
set(ASSIMP_BUILD_LWS_IMPORTER OFF)
set(ASSIMP_BUILD_M3D_IMPORTER OFF)
set(ASSIMP_BUILD_MD2_IMPORTER OFF)
set(ASSIMP_BUILD_MD3_IMPORTER OFF)
set(ASSIMP_BUILD_MD5_IMPORTER OFF)
set(ASSIMP_BUILD_MDC_IMPORTER OFF)
set(ASSIMP_BUILD_MDL_IMPORTER OFF)
set(ASSIMP_BUILD_NFF_IMPORTER OFF)
set(ASSIMP_BUILD_NDO_IMPORTER OFF)
set(ASSIMP_BUILD_OFF_IMPORTER OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_OGRE_IMPORTER OFF)
set(ASSIMP_BUILD_OPENGEX_IMPORTER OFF)
set(ASSIMP_BUILD_PLY_IMPORTER OFF)
set(ASSIMP_BUILD_MS3D_IMPORTER OFF)
set(ASSIMP_BUILD_COB_IMPORTER OFF)
set(ASSIMP_BUILD_BLEND_IMPORTER OFF)
set(ASSIMP_BUILD_IFC_IMPORTER OFF)
set(ASSIMP_BUILD_XGL_IMPORTER OFF)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
set(ASSIMP_BUILD_Q3D_IMPORTER OFF)
set(ASSIMP_BUILD_Q3BSP_IMPORTER OFF)
set(ASSIMP_BUILD_RAW_IMPORTER OFF)
set(ASSIMP_BUILD_SIB_IMPORTER OFF)
set(ASSIMP_BUILD_SMD_IMPORTER OFF)
set(ASSIMP_BUILD_STL_IMPORTER OFF)
set(ASSIMP_BUILD_TERRAGEN_IMPORTER OFF)
set(ASSIMP_BUILD_3D_IMPORTER OFF)
set(ASSIMP_BUILD_X_IMPORTER OFF)
set(ASSIMP_BUILD_X3D_IMPORTER OFF)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)
set(ASSIMP_BUILD_3MF_IMPORTER OFF)
set(ASSIMP_BUILD_MMD_IMPORTER OFF)

message(STATUS "Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

include(gitSubModule)

add_executable(${PROJECT_NAME} src/Poulpe.cpp)

configure_file(PoulpeEngineConfig.h.in PoulpeEngineConfig.h)

add_definitions(-DVK_NO_PROTOTYPES
                -DVOLK_STATIC_DEFINES
                -D_CRT_SECURE_NO_WARNINGS
				-DDWIN32_LEAN_AND_MEAN)

target_precompile_headers(${PROJECT_NAME} PRIVATE include/Poulpepch.hpp)

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
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:PLP_DEBUG_BUILD>")

if(MSVC)
  #set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  set(DEBUG_OPTIONS "/MDd;/Wall;/fp:fast;/std:c++20;/Zc:__cplusplus/-Wno-c++98-compat/-Wno-c++98-compat-pedantic")
  set(RELEASE_OPTIONS "/MD;/Ox;/Ot;/fp:fast;/std:c++20;/Zc:__cplusplus/-Wno-c++98-compat/-Wno-c++98-compat-pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /GL")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
else()
  set(DEBUG_OPTIONS "-Weverything;-std=c++20;-Wno-c++98-compat;-Wno-c++98-compat-pedantic")
  set(RELEASE_OPTIONS "-O3;-std=c++20;-Wno-c++98-compat;-Wno-c++98-compat-pedantic")
endif()

target_compile_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")
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

target_include_directories(${PROJECT_NAME} PUBLIC
					${PROJECT_SOURCE_DIR}/vendor/assimp/include					
					${PROJECT_SOURCE_DIR}/vendor/lua
					${PROJECT_SOURCE_DIR}/vendor/miniaudio					
					${PROJECT_SOURCE_DIR}/vendor/nlohmann/include
                    ${PROJECT_SOURCE_DIR}/vendor/stb
                    ${VULKAN_PATH})

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/assimp)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/nlohmann)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/volk)

target_compile_options(assimp PRIVATE -w -Wno-error)
target_compile_options(assimp_cmd PRIVATE -w -Wno-deprecated-copy)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
target_compile_options(assimp_viewer PRIVATE -w -Wno-error)
endif()
								  
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

if(MSVC)
  list(APPEND EXTRA_LIBS Ws2_32)
endif()

list(APPEND EXTRA_LIBS glfw)
list(APPEND EXTRA_LIBS glm::glm)
list(APPEND EXTRA_LIBS lua)
list(APPEND EXTRA_LIBS nlohmann_json)
list(APPEND EXTRA_LIBS spdlog::spdlog)
list(APPEND EXTRA_LIBS assimp)
list(APPEND EXTRA_LIBS volk)

target_link_libraries(${PROJECT_NAME} PRIVATE ${EXTRA_LIBS} poulpe)
                       
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

#add_custom_command(
#  TARGET ${PROJECT_NAME} PRE_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_directory
#      ${CMAKE_CURRENT_SOURCE_DIR}/assets
#      $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
#  COMMENT "\r\n Copying assets")
#
#add_custom_command(
#  TARGET ${PROJECT_NAME} PRE_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_directory
#      ${CMAKE_CURRENT_SOURCE_DIR}/config
#      $<TARGET_FILE_DIR:${PROJECT_NAME}>/config
#  COMMENT "\r\n Copying config")

#add_custom_command(
#  TARGET ${PROJECT_NAME} PRE_BUILD
#  COMMAND ${CMAKE_COMMAND} -E make_directory 
#      $<TARGET_FILE_DIR:${PROJECT_NAME}>/cache
#  COMMENT "\r\n Creating cache dir")
#
#add_custom_command(
#  TARGET ${PROJECT_NAME} PRE_BUILD
#  COMMAND ${CMAKE_COMMAND} -E make_directory 
#      $<TARGET_FILE_DIR:${PROJECT_NAME}>/lib
#  COMMENT "\r\n Creating lib dir")

add_custom_target(
        CompilShaders ${CMAKE_CURRENT_SOURCE_DIR}/bin/shadersCompilation.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

#add_custom_target(graphviz ALL
#    COMMAND ${CMAKE_COMMAND} "--graphviz=graph/plp.dot" ..
#    COMMAND dot -Tsvg graph/plp.dot -o graph/plp.svg
#    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
#)
#########################################################
