#pragma once

#include <Poulpe/Component/Texture.hpp>

#include <ktx.h>
#include <nlohmann/json.hpp>
#include <stb_image.h>

#include <latch>

namespace Poulpe
{
  class Renderer;

  class TextureManager
  {
  public:
    inline void addConfig(nlohmann::json const& config) { _texture_config = config; }
    std::vector<std::array<float, 3>> addNormalMapTexture(std::string const& name);
    inline void addRenderer(Renderer* renderer) { _renderer = renderer; }
    void addSkyBox(std::vector<std::string> const & skybox_images);
    void addTexture(
      std::string const& name,
      std::string const& path,
      VkFormat const format,
      bool const is_public = false);
    void addKTXTexture(
      std::string const& name,
      std::string const& path,
      ktx_transcode_fmt_e const target_format,
      VkImageAspectFlags const aspect_flags,
      bool const is_public = false);
    void clear();
    inline const Texture getSkyboxTexture() const { return _skybox; }
    inline std::unordered_map<std::string, std::string> getPaths() const { return _paths; }
    inline std::unordered_map<std::string, Texture> getTextures() { return _textures; }
    std::function<void(std::latch& count_down)> load();
    std::function<void(std::latch& count_down)> loadSkybox(std::string_view skybox);

  private:
    void add(
      std::string const& name,
      nlohmann::json const& data,
      VkImageAspectFlags const aspect_flags);

  private:
    const uint32_t MAX_MIPLEVELS = 5;

    Renderer* _renderer{ nullptr };
    Texture _skybox;
    std::string _skybox_name;
    nlohmann::json _texture_config;

    std::unordered_map<std::string, std::string> _paths;
    std::unordered_map<std::string, Texture> _textures;
  };
}
