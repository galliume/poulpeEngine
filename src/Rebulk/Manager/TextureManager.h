#pragma once
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"

namespace Rbk
{
    class TextureManager
    {
    public:
        explicit TextureManager(std::shared_ptr<VulkanRenderer> renderer);
        void AddSkyBox(std::vector<std::string> skyboxImages);
        inline Texture GetSkyboxTexture() { return m_Skybox; };
        void AddTexture(std::string name, std::string path);
        inline std::map<std::string, Texture> GetTextures() { return m_Textures; };
        void Load();
        inline std::map<std::string, std::string> GetPaths() { return m_Paths; };

    private:
        std::map<std::string, Texture> m_Textures;
        Texture m_Skybox;
        std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;
        std::map<std::string, std::string>m_Paths;
    };
}