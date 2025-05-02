#pragma once

#include <Poulpe/Component/Texture.hpp>

#include <ktx.h>
#include <nlohmann/json.hpp>
#include <stb_image.h>

#include <latch>

namespace Poulpe
{
  class Renderer;

  enum class TEXTURE_TYPE {
    DIFFUSE,
    NORMAL,
    MR,//metalRoughness
    EMISSIVE,
    AO,//ambientOcclusion,
    HEIGHT
  };

  class TextureManager
  {
  public:
    inline void addConfig(nlohmann::json const& config) { _texture_config = config; }
    std::vector<std::array<float, 3>> addNormalMapTexture(std::string const& name);
    inline void addRenderer(Renderer* renderer) { _renderer = renderer; }
    void addSkyBox(
      std::string const& skybox_name,
      std::vector<std::string> const & skybox_images);
    void addKTXTexture(
      std::string const& name,
      std::string const& path,
      VkImageAspectFlags const aspect_flags,
      ktx_transcode_fmt_e const transcoding,
      bool const is_public = false);
    void clear();
    inline Texture getSkyboxTexture() { return _textures[_skybox_name]; }
    inline std::unordered_map<std::string, std::string> getPaths() const { return _paths; }
    inline std::unordered_map<std::string, Texture> getTextures() { return _textures; }
    std::function<void(std::latch& count_down)> load();
    std::function<void(std::latch& count_down)> loadSkybox(std::string_view skybox);
    Texture getTerrainTexture() { return _textures[_terrain_name]; }
    void setTerrainName(std::string const& terrain_name) { _terrain_name = terrain_name; }
    void setWaterName(std::string const& water_name) { _water_name = water_name; }
    Texture getWaterTexture() { return _textures[_water_name]; }
    void addTexture(Texture texture) { _textures[texture.getName()] = std::move(texture); }

  private:
    void add(
      std::string const& name,
      nlohmann::json const& data,
      VkImageAspectFlags const aspect_flags,
      TEXTURE_TYPE texture_type);

  private:
    const uint32_t MAX_MIPLEVELS = 5;

    Renderer* _renderer{ nullptr };
    std::string _skybox_name;
    std::string _terrain_name {"_plp_terrain"};
    std::string _water_name {"_plp_water"};
    nlohmann::json _texture_config;

    std::unordered_map<std::string, std::string> _paths;
    std::unordered_map<std::string, Texture> _textures;
  };
}
