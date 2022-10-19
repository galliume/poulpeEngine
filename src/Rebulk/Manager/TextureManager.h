#pragma once

#include "ITextureManager.h"

namespace Rbk
{
    class TextureManager : ITextureManager
    {
    public:
        explicit TextureManager();
        
        virtual void AddConfig(nlohmann::json config) override { m_TextureConfig = config; } ;
        virtual inline std::map<std::string, Texture> GetTextures() const override  { return m_Textures; };
        virtual void AddRenderer(std::shared_ptr<IRendererAdapter> renderer) override { m_Renderer = renderer; };
        virtual std::vector<std::future<void>> Load(const std::string& skybox) override;
        virtual inline const Texture GetSkyboxTexture() const override { return m_Skybox; };
        virtual void AddSkyBox(const std::vector<std::string>& skyboxImages) override;
        virtual void AddTexture(const std::string& name, const std::string& path, bool isPublic = false) override;
        virtual inline std::map<std::string, std::string> GetPaths() const override { return m_Paths; };

    private:
        std::map<std::string, Texture> m_Textures;
        std::map<std::string, std::string> m_Paths;
        std::shared_ptr<IRendererAdapter> m_Renderer = nullptr;
        nlohmann::json m_TextureConfig;

        Texture m_Skybox;
    };
}