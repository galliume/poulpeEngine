#pragma once

#include "Rebulk/Component/Mesh.hpp"
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.hpp"

namespace Rbk
{
    class DestroyManager
    {
    public:
        DestroyManager();

        //@todo should not be specific impl
        void SetRenderer(std::shared_ptr<VulkanRenderer> renderer);
        void AddMemoryPool(std::shared_ptr<DeviceMemoryPool> deviceMemoryPool) { m_DeviceMemoryPool = deviceMemoryPool; }
        void CleanEntities(std::vector<std::shared_ptr<Entity>> entities);
        void CleanEntities(std::vector<std::shared_ptr<Mesh>> entities);
        void CleanEntity(std::shared_ptr<Mesh> entity);
        void CleanShaders(std::map<std::string, std::array<VkShaderModule, 2>> shaders);
        void CleanTextures(std::map<std::string, Texture> textures);
        void CleanTexture(Texture textures);
        void CleanDeviceMemory();

    private:
        std::shared_ptr<VulkanRenderer> m_Renderer;
        std::shared_ptr<DeviceMemoryPool> m_DeviceMemoryPool;
    };
}
