#pragma once

#include "ITextureManager.hpp"

namespace Poulpe
{
    class TextureManager : public ITextureManager
    {
    public:
        TextureManager() = default;
        ~TextureManager() override = default;

        inline void addConfig(nlohmann::json config) override { m_TextureConfig = config; }
        Texture addNormalMapTexture(std::string const& name) override;
        inline void addRenderer(IRenderer* renderer) override { m_Renderer = renderer; }
        void addSkyBox(std::vector<std::string> const & skyboxImages) override;
        void addTexture(std::string const & name, std::string const & path, bool isPublic = false, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB, int stbiChannel = STBI_rgb_alpha) override;
        void clear();
        inline const Texture getSkyboxTexture() const override { return m_Skybox; }
        inline std::unordered_map<std::string, std::string> getPaths() const override { return m_Paths; }
        inline std::unordered_map<std::string, Texture> getTextures() override { return m_Textures; }
        inline bool isSkyboxLoadingDone() { return m_SkyboxLoadingDone.load(); }
        inline bool isTexturesLoadingDone() { return m_TexturesLoadingDone.load(); }
        std::function<void()> load() override;
        std::function<void()> loadSkybox(std::string_view skybox) override;

    private:
        const uint32_t MAX_MIPLEVELS = 5;

        IRenderer* m_Renderer{ nullptr };
        Texture m_Skybox;
        std::string m_SkyboxName;
        nlohmann::json m_TextureConfig;

        std::unordered_map<std::string, std::string> m_Paths;
        std::unordered_map<std::string, Texture> m_Textures;

        std::atomic_bool m_SkyboxLoadingDone{ false };
        std::atomic_bool m_TexturesLoadingDone{ false };
    };
}
