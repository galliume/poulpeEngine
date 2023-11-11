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
#set(CMAKE_CXX_CLANG_TIDY clang-tidy)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(gcc_like_c "$<COMPILE_LANG_AND_ID:C,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(msvc_c "$<COMPILE_LANG_AND_ID:C,MSVC>")
set(GIT_SUBMODULE ON)
set(GLFW_LIBRARY_TYPE STATIC)

set(VOLK_PULL_IN_VULKAN false)
#weird bug with vulkan loader ?
set(CMAKE_MSVC_RUNTIME_LIBRARY "")
#need to find how to have wayland on github actions...
set(GLFW_BUILD_WAYLAND OFF)
set(GLFW_BUILD_X11 ON)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)

message(STATUS "Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

include(gitSubModule)

configure_file(PoulpeEngineConfig.h.in PoulpeEngineConfig.h)

add_definitions(-DVK_NO_PROTOTYPES
                -DVOLK_STATIC_DEFINES
                -DIMGUI_IMPL_VULKAN_NO_PROTOTYPES
                -D_CRT_SECURE_NO_WARNINGS)

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

include_directories(${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/vendor/glfw/include
                    ${PROJECT_SOURCE_DIR}/vendor/spdlog/include
                    ${PROJECT_SOURCE_DIR}/vendor/glm
                    ${PROJECT_SOURCE_DIR}/vendor/stb
                    ${PROJECT_SOURCE_DIR}/vendor/nlohmann/include
                    ${PROJECT_SOURCE_DIR}/vendor/imgui
                    ${VULKAN_PATH})

add_executable(${PROJECT_NAME} src/Poulpe.cpp)

target_precompile_headers(${PROJECT_NAME} PRIVATE include/Poulpepch.hpp)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Component)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Core)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/GUI)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/GUI/ImGui)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/GUI/Layer)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Manager)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Renderer)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Renderer/Adapter)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Renderer/Vulkan)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Poulpe/Renderer/Vulkan/Entities)

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/volk)

add_library(dearimgui STATIC ${PROJECT_SOURCE_DIR}/vendor/imgui/imconfig.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui.cpp
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_internal.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imstb_rectpack.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imstb_textedit.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imstb_truetype.h
				  ${PROJECT_SOURCE_DIR}/vendor/imgui/imstb_truetype.h
				  ${PROJECT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.h
				  ${PROJECT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp
				  ${PROJECT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_vulkan.h
				  ${PROJECT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_vulkan.cpp
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp)

add_library(miniaudio STATIC 
	${PROJECT_SOURCE_DIR}/vendor/miniaudio/extras/miniaudio_split/miniaudio.h
	${PROJECT_SOURCE_DIR}/vendor/miniaudio/extras/miniaudio_split/miniaudio.c)

add_library(tiny_obj_loader STATIC 
	${PROJECT_SOURCE_DIR}/vendor/tiny_obj_loader/tiny_obj_loader.h
	${PROJECT_SOURCE_DIR}/vendor/tiny_obj_loader/tiny_obj_loader.cc)

list(APPEND EXTRA_LIBS glfw)
list(APPEND EXTRA_LIBS glm::glm)
list(APPEND EXTRA_LIBS volk)
list(APPEND EXTRA_LIBS dearimgui)
list(APPEND EXTRA_LIBS miniaudio)
list(APPEND EXTRA_LIBS tiny_obj_loader)

target_link_libraries(${PROJECT_NAME} PRIVATE ${EXTRA_LIBS} poulpe)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}")

target_include_directories(${PROJECT_NAME} PRIVATE
                          "${PROJECT_SOURCE_DIR}/vendor/glfw"
                          "${PROJECT_SOURCE_DIR}/vendor/glm"
                          "${PROJECT_SOURCE_DIR}/vendor/volk"
                          "${PROJECT_SOURCE_DIR}/vendor/imgui"
						  "${PROJECT_SOURCE_DIR}/vendor/miniaudio/extras/miniaudio_split"
						  "${PROJECT_SOURCE_DIR}/vendor/tiny_obj_loader")

target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE
                          "${PROJECT_SOURCE_DIR}/vendor/glfw"
                          "${PROJECT_SOURCE_DIR}/vendor/glm"
                          "${PROJECT_SOURCE_DIR}/vendor/volk"
                          "${PROJECT_SOURCE_DIR}/vendor/imgui"
						  "${PROJECT_SOURCE_DIR}/vendor/miniaudio/extras/miniaudio_split"
						  "${PROJECT_SOURCE_DIR}/vendor/tiny_obj_loader")

target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:PLP_DEBUG_BUILD>")

if(MSVC)
	set(DEBUG_OPTIONS "/WX;/W4")
	set(RELEASE_OPTIONS "-O2")
else()
	set(DEBUG_OPTIONS "-Wextra -Wshadow -Wformat=2 -Wunused -Wconversion -Werror")
	set(RELEASE_OPTIONS "-O2")
endif()

target_compile_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PRIVATE "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")

target_compile_options(dearimgui PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W0 /O2>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w -O2>
)

target_compile_options(miniaudio PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W0 /O2>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w -O2>
)

target_compile_options(tiny_obj_loader PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W0 /O2>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w -O2>
)

#Filters for visual studio structure
file(GLOB_RECURSE _source_list "${PROJECT_SOURCE_DIR}/src/**/*.cpp" "${PROJECT_SOURCE_DIR}/src/**/*.hpp")
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
  list(APPEND TIDY_SRC "${_source}")
endforeach()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL$<$<CONFIG:Debug>:Debug>")

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

#target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
#target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

 add_custom_command(
         TARGET ${PROJECT_NAME} PRE_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory
                 ${CMAKE_CURRENT_SOURCE_DIR}/assets
                 $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
		 COMMENT "Copying assets")

add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/config
                $<TARGET_FILE_DIR:${PROJECT_NAME}>/config
		COMMENT "Copying config")
				
add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory 
				$<TARGET_FILE_DIR:${PROJECT_NAME}>/cache
		COMMENT "Creating cache dir")
		
add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory 
				$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib
		COMMENT "Creating lib dir")

set_target_properties(dearimgui PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib"
)
set_target_properties(miniaudio PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib"
)		
set_target_properties(glfw PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib"
)
set_target_properties(volk PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib"
)
set_target_properties(tiny_obj_loader PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>/lib"
)