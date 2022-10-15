#pragma once
#include "Rebulk/Renderer/Vulkan/VulkanRenderer.h"
#include "json.hpp"

namespace Rbk
{
    class TextureManager
    {
    public:
        explicit TextureManager(
            std::shared_ptr<VulkanRenderer> renderer,
            nlohmann::json textureConfig);
        
        void AddSkyBox(const std::vector<std::string>& skyboxImages);
        void AddTexture(const std::string& name, const std::string& path);
        std::vector<std::future<void>> Load(const std::string& skybox);

        inline const Texture GetSkyboxTexture() const { return m_Skybox; };
        inline std::map<std::string, Texture> GetTextures() const { return m_Textures; };
        inline std::map<std::string, std::string> GetPaths() const { return m_Paths; };

    private:
        std::map<std::string, Texture> m_Textures;
        std::map<std::string, std::string> m_Paths;
        std::shared_ptr<VulkanRenderer> m_Renderer = nullptr;
        nlohmann::json m_TextureConfig;

        Texture m_Skybox;
    };
}