target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES  
      FILES
        DeviceMemory.cppm
        DeviceMemoryPool.cppm
        Renderer.cppm
        VulkanAPI.cppm
  PRIVATE
    DeviceMemory.cpp
    DeviceMemoryPool.cpp
    Renderer.cpp
    VulkanAPI.cpp)