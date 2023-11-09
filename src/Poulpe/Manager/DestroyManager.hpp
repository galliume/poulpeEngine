#pragma once

#include "Poulpe/Component/Mesh.hpp"
#include "Poulpe/Renderer/Vulkan/VulkanRenderer.hpp"

namespace Poulpe
{
    class DestroyManager
    {
    public:
        DestroyManager();

        //@todo should not be specific impl
        void setRenderer(std::shared_ptr<VulkanRenderer> renderer);
        void addMemoryPool(std::shared_ptr<DeviceMemoryPool> deviceMemoryPool) { m_DeviceMemoryPool = deviceMemoryPool; }
        void cleanEntities(std::vector<std::shared_ptr<Entity>> entities);
        void cleanEntities(std::vector<std::shared_ptr<Mesh>> entities);
        void cleanEntity(std::shared_ptr<Mesh> entity);
        void cleanShaders(std::unordered_map<std::string, std::array<VkShaderModule, 2>> shaders);
        void cleanTextures(std::unordered_map<std::string, Texture> textures);
        void cleanTexture(Texture textures);
        void cleanDeviceMemory();

    private:
        std::shared_ptr<VulkanRenderer> m_Renderer;
        std::shared_ptr<DeviceMemoryPool> m_DeviceMemoryPool;
    };
}
