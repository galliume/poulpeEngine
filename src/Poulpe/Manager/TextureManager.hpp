
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
        void addRenderer(IRendererAdapter* renderer) override { m_Renderer = renderer; }
        std::function<void()> load() override;
        std::function<void()> loadSkybox(std::string_view skybox) override;
        inline const Texture getSkyboxTexture() const override { return m_Skybox; }
        void addSkyBox(std::vector<std::string> const & skyboxImages) override;
        void addTexture(std::string const & name, std::string const & path, bool isPublic = false) override;
        std::vector<std::array<float, 3>> addNormalMapTexture(std::string const& name) override;
        inline std::unordered_map<std::string, std::string> getPaths() const override { return m_Paths; }
        void clear();
        bool isTexturesLoadingDone() { return m_TexturesLoadingDone.load(); }
        bool isSkyboxLoadingDone() { return m_SkyboxLoadingDone.load(); }

    private:
        std::unordered_map<std::string, Texture> m_Textures;
        std::unordered_map<std::string, std::string> m_Paths;
        IRendererAdapter* m_Renderer = nullptr;
        nlohmann::json m_TextureConfig;
        std::atomic_bool m_TexturesLoadingDone{ false };
        std::atomic_bool m_SkyboxLoadingDone{ false };
        std::string m_SkyboxName;

        Texture m_Skybox;

        const int MAX_MIPLEVELS = 5;
    };
}
