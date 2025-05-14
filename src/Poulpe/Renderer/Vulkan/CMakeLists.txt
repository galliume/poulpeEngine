target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Vulkan.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/DeviceMemory.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/DeviceMemoryPool.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/VulkanAPI.cppm
        ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cppm
  PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/DeviceMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DeviceMemoryPool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/VulkanAPI.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cpp)