#pragma once

#include "ITextureManager.hpp"

namespace Rbk
{
    class TextureManager : ITextureManager
    {
    public:
        explicit TextureManager();
        ~TextureManager();

        virtual void AddConfig(nlohmann::json config) override { m_TextureConfig = config; }
        virtual inline std::map<std::string, Texture> GetTextures() override { return m_Textures; }
        virtual void AddRenderer(std::shared_ptr<IRendererAdapter> renderer) override { m_Renderer = renderer; }
        virtual std::function<void()> Load() override;
        virtual std::function<void()> LoadSkybox(const std::string& skybox) override;
        virtual inline const Texture GetSkyboxTexture() const override { return m_Skybox; }
        virtual void AddSkyBox(const std::vector<std::string>& skyboxImages) override;
        virtual void AddTexture(const std::string& name, const std::string& path, bool isPublic = false) override;
        virtual inline std::map<std::string, std::string> GetPaths() const override { return m_Paths; }
        void Clear();
        bool IsTexturesLoadingDone() { return m_TexturesLoadingDone.load(); }
        bool IsSkyboxLoadingDone() { return m_SkyboxLoadingDone.load(); }

    private:
        std::map<std::string, Texture> m_Textures;
        std::map<std::string, std::string> m_Paths;
        std::shared_ptr<IRendererAdapter> m_Renderer = nullptr;
        nlohmann::json m_TextureConfig;
        std::atomic_bool m_TexturesLoadingDone = false;
        std::atomic_bool m_SkyboxLoadingDone = false;
        std::string m_SkyboxName;

        Texture m_Skybox;
    };
}
