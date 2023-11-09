
#pragma once

#include "ITextureManager.hpp"

namespace Poulpe
{
    class TextureManager : ITextureManager
    {
    public:
        TextureManager() = default;
        virtual ~TextureManager() = default;

        void addConfig(nlohmann::json config) override { m_TextureConfig = config; }
        inline std::unordered_map<std::string, Texture> getTextures() override { return m_Textures; }
        void addRenderer(std::shared_ptr<IRendererAdapter> renderer) override { m_Renderer = renderer; }
        std::function<void()> load(std::condition_variable& cv) override;
        std::function<void()> loadSkybox(std::string_view skybox, std::condition_variable& cv) override;
        inline const Texture getSkyboxTexture() const override { return m_Skybox; }
        void addSkyBox(const std::vector<std::string>& skyboxImages) override;
        void addTexture(const std::string& name, const std::string& path, bool isPublic = false) override;
        inline std::unordered_map<std::string, std::string> getPaths() const override { return m_Paths; }
        void clear();
        bool isTexturesLoadingDone() { return m_TexturesLoadingDone.load(); }
        bool isSkyboxLoadingDone() { return m_SkyboxLoadingDone.load(); }

    private:
        std::unordered_map<std::string, Texture> m_Textures;
        std::unordered_map<std::string, std::string> m_Paths;
        std::shared_ptr<IRendererAdapter> m_Renderer = nullptr;
        nlohmann::json m_TextureConfig;
        std::atomic_bool m_TexturesLoadingDone{ false };
        std::atomic_bool m_SkyboxLoadingDone{ false };
        std::string m_SkyboxName;

        Texture m_Skybox;

        const int MAX_MIPLEVELS = 5;
    };
}
