#pragma once

#include "Poulpe/Component/Mesh.hpp"

#include "Poulpe/Renderer/Vulkan/VulkanRenderer.hpp"

namespace Poulpe
{
    class DestroyManager
    {
    public:
        DestroyManager() = default;

        //@todo should not be specific impl
        void setRenderer(VulkanRenderer* renderer);
        void addMemoryPool(DeviceMemoryPool* deviceMemoryPool) { m_DeviceMemoryPool = deviceMemoryPool; }

        template<std::derived_from<Entity> T>
        void cleanEntities(std::vector<std::unique_ptr<T>> & entities);

        template<std::derived_from<Entity> T>
        void cleanEntities(std::vector<T*> & entities);

        template<std::derived_from<Entity> T>
        void cleanEntity(T* entity);

        void cleanShaders(std::unordered_map<std::string, std::array<VkShaderModule, 2>> shaders);
        void cleanTextures(std::unordered_map<std::string, Texture> textures);
        void cleanTexture(Texture textures);
        void cleanDeviceMemory();

    private:
        VulkanRenderer* m_Renderer;
        DeviceMemoryPool* m_DeviceMemoryPool;
    };
}
