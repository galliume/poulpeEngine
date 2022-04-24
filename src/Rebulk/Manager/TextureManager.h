#pragma once
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"

namespace Rbk
{
    class TextureManager
    {
    public:
        TextureManager(VulkanRenderer* renderer);
        void AddSkyBox(std::vector<const char*> skyboxImages);
        inline Texture GetSkyboxTexture() { return m_Skybox; };
        void AddTexture(const char* name, const char* path);
        inline std::map<const char*, Texture> GetTextures() { return m_Textures; };

    private:
        std::map<const char*, Texture> m_Textures;
        Texture m_Skybox;
        VulkanRenderer* m_Renderer = nullptr;
    };
}