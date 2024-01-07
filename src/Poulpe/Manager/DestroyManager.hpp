#pragma once

#include "Poulpe/Component/Entity.hpp"
#include "Poulpe/Component/Mesh.hpp"

#include "Poulpe/Renderer/IRenderer.hpp"

//@TODO refactor all destroy system...
namespace Poulpe
{
    class DestroyManager
    {
    public: 
        DestroyManager() = default;

        inline void addMemoryPool(DeviceMemoryPool* const deviceMemoryPool) { m_DeviceMemoryPool = deviceMemoryPool; }
        void cleanDeviceMemory();

        template<std::derived_from<Entity> T>
        void cleanEntity(T* entity);

        template<std::derived_from<Entity> T>
        void cleanEntities(std::vector<std::unique_ptr<T>> & entities);

        template<std::derived_from<Entity> T>
        void cleanEntities(std::vector<T*> & entities);

        void cleanShaders(std::unordered_map<std::string, std::vector<VkShaderModule>> shaders);
        void cleanTexture(Texture textures);
        void cleanTextures(std::unordered_map<std::string, Texture> textures);
        void setRenderer(IRenderer* renderer);

    private:
        DeviceMemoryPool* m_DeviceMemoryPool;
        IRenderer* m_Renderer;
    };
}
