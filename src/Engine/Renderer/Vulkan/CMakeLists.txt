add_library(Vulkan STATIC)

target_sources(Vulkan
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES
      FILES
        DeviceMemory.cppm
        DeviceMemoryPool.cppm
        Renderer.cppm
        RendererComponentTypes.cppm
        VulkanAPI.cppm
        RendererComponent.cppm

        Component/Basic.cppm
        Component/Skybox.cppm
        Component/ShadowMap.cppm
        Component/Terrain.cppm
        Component/Text.cppm
        Component/Water.cppm

  PRIVATE
    DeviceMemory.cpp
    DeviceMemoryPool.cpp
    Renderer.cpp
    VulkanAPI.cpp

    Component/Basic.cpp
    Component/Skybox.cpp
    Component/ShadowMap.cpp
    Component/Terrain.cpp
    Component/Text.cpp
    Component/Water.cpp)

target_link_libraries(
  Vulkan

  PRIVATE
    Components
    Core
    EngineCommon

  PUBLIC
    glfw
    ktx
    stb)

set_poulpe_definitions(Vulkan)
set_poulpe_links(Vulkan)
set_poulpe_compiler_flags(Vulkan)