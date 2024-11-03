#pragma once

#include <Poulpe/Component/Texture.hpp>

#include <nlohmann/json.hpp>
#include <stb_image.h>

#include <latch>

namespace Poulpe
{
  class Renderer;

  class TextureManager
  {
  public:
    TextureManager() = default;
    ~TextureManager() = default;

    inline void addConfig(nlohmann::json config) { m_TextureConfig = config; }
    std::vector<std::array<float, 3>> addNormalMapTexture(std::string const& name);
    inline void addRenderer(Renderer* renderer) { m_Renderer = renderer; }
    void addSkyBox(std::vector<std::string> const & skyboxImages);
    void addTexture(std::string const & name, std::string const & path, bool isPublic = false);
    void clear();
    inline const Texture getSkyboxTexture() const { return m_Skybox; }
    inline std::unordered_map<std::string, std::string> getPaths() const { return m_Paths; }
    inline std::unordered_map<std::string, Texture> getTextures() { return m_Textures; }
    inline bool isSkyboxLoadingDone() { return m_SkyboxLoadingDone.load(); }
    inline bool isTexturesLoadingDone() { return m_TexturesLoadingDone.load(); }
    std::function<void(std::latch& count_down)> load();
    std::function<void(std::latch& count_down)> loadSkybox(std::string_view skybox);

  private:
    const uint32_t MAX_MIPLEVELS = 5;

    Renderer* m_Renderer{ nullptr };
    Texture m_Skybox;
    std::string m_SkyboxName;
    nlohmann::json m_TextureConfig;

    std::unordered_map<std::string, std::string> m_Paths;
    std::unordered_map<std::string, Texture> m_Textures;

    std::atomic_bool m_SkyboxLoadingDone{ false };
    std::atomic_bool m_TexturesLoadingDone{ false };
  };
}
