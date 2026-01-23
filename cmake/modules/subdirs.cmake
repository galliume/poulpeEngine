message(STATUS "Setting include.")

target_include_directories(${PROJECT_NAME}
PRIVATE
  ${PROJECT_SOURCE_DIR}/src)

set(ENGINE_SUBDIRS
  Engine
  Engine/Animation
  Engine/Component
  Engine/Core
  Engine/GUI
  Engine/Manager
  Engine/Network
  Engine/Renderer
  Engine/Renderer/Vulkan
  Engine/Utils
  Editor
  Editor/Manager
)

foreach(subdir ${ENGINE_SUBDIRS})
  add_subdirectory(${PROJECT_SOURCE_DIR}/src/${subdir})
endforeach()

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})

message(STATUS "includes OK.")