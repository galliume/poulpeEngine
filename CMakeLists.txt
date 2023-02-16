cmake_minimum_required(VERSION 3.26)

project(Rebulkan
  VERSION 0.0.1
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

target_compile_options(rebulkan_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
)

configure_file(RebulkanConfig.h.in RebulkanConfig.h)

add_executable(Rebulkan src/Rebulkan.cpp)

include_directories(vendor/glfw vendor/spdlog vendor/glm vendor/stb_image vendor/miniaudio vendor/nlohmann vendor/tiny_obj_loader vendor/volk vendor/imgui)

add_subdirectory(src)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/glm)

if (DEFINED ENV{VULKAN_SDK}) 
	string(CONCAT VULKAN_PATH $ENV{VULKAN_SDK} "/Include")
	include_directories(${VULKAN_PATH})
else()
	message(FATAL_ERROR "The env var VULKAN_SDK not found.")
endif()
	

list(APPEND EXTRA_LIBS glfw)
list(APPEND EXTRA_LIBS spdlog)
list(APPEND EXTRA_LIBS glm::glm)

target_link_libraries(Rebulkan PUBLIC ${EXTRA_LIBS} rebulkan_compiler_flags)

target_include_directories(Rebulkan PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
						   
#set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})


