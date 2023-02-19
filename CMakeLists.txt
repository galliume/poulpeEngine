cmake_minimum_required(VERSION 3.25.2)

project(Rebulkan
  VERSION 0.1.1
  DESCRIPTION "Game Engine with a vulkan renderer"
  HOMEPAGE_URL "https://github.com/galliume/rebulkan"
  LANGUAGES CXX
)

add_library(rebulkan_compiler_flags INTERFACE)
 
target_compile_features(rebulkan_compiler_flags INTERFACE cxx_std_20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED            ON)
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(GIT_SUBMODULE ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Rebulkan)

message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
message("CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message("CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

configure_file(RebulkanConfig.h.in RebulkanConfig.h)

include(gitSubModule)

target_compile_options(rebulkan_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

configure_file(RebulkanConfig.h.in RebulkanConfig.h)

add_definitions(-DVK_NO_PROTOTYPES 
                -DVOLK_STATIC_DEFINES 
                -DIMGUI_IMPL_VULKAN_NO_PROTOTYPES)

if (DEFINED ENV{VULKAN_SDK}) 
  cmake_path(SET VULKAN_PATH "$ENV{VULKAN_SDK}/Include")
  message("Found Vulkan include dir: ${VULKAN_PATH}")  
  find_package(Vulkan REQUIRED)
else()
  message(FATAL_ERROR "The env var VULKAN_SDK not found.")
endif()

include_directories(${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/vendor/glfw/include
                    ${PROJECT_SOURCE_DIR}/vendor/spdlog/include 
                    ${PROJECT_SOURCE_DIR}/vendor/glm 
                    ${PROJECT_SOURCE_DIR}/vendor/stb 
                    ${PROJECT_SOURCE_DIR}/vendor/miniaudio 
                    ${PROJECT_SOURCE_DIR}/vendor/nlohmann/include 
                    ${PROJECT_SOURCE_DIR}/vendor/tiny_obj_loader
                    ${PROJECT_SOURCE_DIR}/vendor/imgui)

add_executable(Rebulkan src/Rebulkan.cpp include/build.cpp)

add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk/Component)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk/Core)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk/GUI)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk/GUI/ImGui)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk/GUI/Layer)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk/Manager)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk/Renderer)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk/Renderer/Adapter)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk/Renderer/Vulkan)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/Rebulk/Renderer/Vulkan/Meshator)

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/volk)

add_library(ImGui ${PROJECT_SOURCE_DIR}/vendor/imgui/imconfig.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui.cpp
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_internal.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imstb_rectpack.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imstb_textedit.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imstb_truetype.h
                  ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp)
   
list(APPEND EXTRA_LIBS glfw)
list(APPEND EXTRA_LIBS spdlog)
list(APPEND EXTRA_LIBS glm::glm)
list(APPEND EXTRA_LIBS volk)
list(APPEND EXTRA_LIBS ImGui)
list(APPEND EXTRA_LIBS Vulkan::Vulkan)

target_precompile_headers(Rebulkan PUBLIC include/rebulkpch.hpp)

target_link_libraries(Rebulkan PRIVATE ${EXTRA_LIBS} rebulkan_compiler_flags)

target_include_directories(Rebulkan PUBLIC "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/vendor/glfw"
                          "${PROJECT_SOURCE_DIR}/vendor/spdlog"
                          "${PROJECT_SOURCE_DIR}/vendor/glm"
                          "${PROJECT_SOURCE_DIR}/vendor/volk"
                          "${PROJECT_SOURCE_DIR}/vendor/imgui")
					
#Filters for visual studio structure					
file(GLOB_RECURSE _source_list *.cpp* *.h* *.hpp*)
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()						  

set_property(TARGET Rebulkan PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:Rebulkan>)

add_custom_command(TARGET Rebulkan 
				   PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/config/ $<TARGET_FILE_DIR:Rebulkan>/config)
				   
add_custom_command(TARGET Rebulkan 
				   PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:Rebulkan>/assets)				   

add_custom_command(TARGET Rebulkan
				   PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Rebulkan>/cache)
				   
add_custom_command(TARGET Rebulkan 
				   PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/imgui.ini $<TARGET_FILE_DIR:Rebulkan>/imgui.ini)
				