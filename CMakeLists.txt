##################### PoulpeEngine Project ##############

cmake_minimum_required(VERSION 3.31.0)

#https://www.kitware.com/import-std-in-cmake-3-30/
#set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(PoulpeEngine
  VERSION 0.2.1
  DESCRIPTION "Game Engine with a vulkan renderer"
  HOMEPAGE_URL "https://github.com/galliume/poulpeEngine"
  LANGUAGES C CXX
  )

add_executable(${PROJECT_NAME} src/Poulpe.cpp poulpeEngine.rc)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()


add_library(poulpe INTERFACE)

message(STATUS "Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Build type" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

configure_file(PoulpeEngineConfig.h.in PoulpeEngineConfig.h)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine/Animation)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine/Component)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine/Core)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine/Core/Network)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine/GUI)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine/Manager)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine/Renderer)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine/Renderer/Vulkan)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine/Renderer/Vulkan/Component)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Engine/Utils)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/Editor)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Editor/Manager)

set(DEBUG_OPTIONS
  -g
  -O0
  -pthread
  -femulated-tls
  -Weverything
  -fno-inline -fno-inline-functions
  -fno-omit-frame-pointer
  -fno-strict-aliasing
  -Wno-c++98-compat
  -Wno-old-style-cast
  -Wno-padded
  -Wno-documentation
  -Wno-documentation-unknown-command
  -Wno-decls-in-multiple-modules
  -Wno-unsafe-buffer-usage
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-covered-switch-default
  -Wno-deprecated-declarations)

set(RELEASE_OPTIONS
  -O3
  -pthread
  -femulated-tls
  -march=x86-64
  -DNDEBUG
  -Weverything
  -Wno-c++98-compat
  -Wno-old-style-cast
  -Wno-padded
  -Wno-documentation
  -Wno-documentation-unknown-command
  -Wno-decls-in-multiple-modules
  -Wno-unsafe-buffer-usage
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-covered-switch-default
  -Wno-deprecated-declarations)

#include(CheckIPOSupported)
#check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
#if(ipo_supported)
#  message(STATUS "ipo_supported")
#  set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
#endif()

set(LINK_OPTIONS
  -pthread
  -femulated-tls)

add_definitions(
  -DVK_NO_PROTOTYPES
  -D_USE_MATH_DEFINES)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_definitions(
    -DVOLK_STATIC_DEFINES
    -D_CRT_SECURE_NO_WARNINGS
    -DDWIN32_LEAN_AND_MEAN)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 23
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS ON)

target_compile_features(${PROJECT_NAME}
  PRIVATE   cxx_std_23
  INTERFACE cxx_std_23)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})

target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:${LINK_OPTIONS}>)
target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:${LINK_OPTIONS}>)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:PLP_DEBUG_BUILD>)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  list(APPEND EXTRA_LIBS
    Ws2_32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND EXTRA_LIBS Threads::Threads)

find_package(Vulkan REQUIRED)
list(APPEND EXTRA_LIBS Vulkan::Vulkan)

set(PLP_LIB_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib")
set(PLP_BIN_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

include(fetchDeps)

message(STATUS "EXTRA_LIBS: ${EXTRA_LIBS}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${EXTRA_LIBS})

install(TARGETS ${PROJECT_NAME}
    EXPORT PoulpeEngineTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    FILE_SET cxx_headers DESTINATION include
    FILE_SET cxx_modules DESTINATION include)

find_package(TCL 9 REQUIRED)

message(STATUS "TCL_INCLUDE_PATH = ${TCL_INCLUDE_PATH}")
message(STATUS "TCL_LIBRARY = ${TCL_LIBRARY}")
message(STATUS "TCL_TCLSH = ${TCL_TCLSH}")

#todo: add flag to activate or not the tcl/tk editor
target_include_directories(${PROJECT_NAME} PRIVATE ${TCL_INCLUDE_PATH} ${TK_INCLUDE_PATH})
target_link_libraries(${PROJECT_NAME} PRIVATE ${TCL_LIBRARY} ${TK_LIBRARY})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
  COMMAND_EXPAND_LISTS
)

##################### Install ###########################

# install(TARGETS
#   ${PROJECT_NAME}
# RUNTIME
#   COMPONENT Runtime
# LIBRARY
#   COMPONENT Runtime
#   NAMELINK_COMPONENT Development
# ARCHIVE
#   COMPONENT Development
#   DESTINATION lib/static
# FILE_SET HEADERS
#   COMPONENT Development
# )

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

add_custom_target(
        CompilShaders ${CMAKE_CURRENT_SOURCE_DIR}/bin/shadersCompilation.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

include(graphVizOptions)
