module;

#include <ktx.h>
#include <nlohmann/json.hpp>
#include <stb_image.h>
#include <volk.h>

#include <cstdlib>
#include <filesystem>
#include <latch>
#include <string>
#include <vector>

export module Poulpe.Managers:TextureManager;

import Poulpe.Component.Texture;
import Poulpe.Renderer;

namespace Poulpe
{
  export enum class TEXTURE_TYPE {
    DIFFUSE,
    NORMAL,
    MR,//metalRoughness
    EMISSIVE,
    AO,//ambientOcclusion,
    HEIGHT
  };

  export class TextureManager
  {
  public:
    inline void addConfig(nlohmann::json const& config) { _texture_config = config; }
    std::vector<std::array<float, 3>> addNormalMapTexture(std::string const& name);
    void addSkyBox(
      std::string const& skybox_name,
      std::vector<std::string> const & skybox_images,
      Renderer* const renderer);
    void addKTXTexture(
      std::string const& name,
      std::string const& path,
      VkImageAspectFlags const aspect_flags,
      ktx_transcode_fmt_e const transcoding,
      Renderer *const renderer,
      bool const is_public = false);
    void clear();
    std::string getSkyboxTexture() const& {_skybox_name; }
    std::unordered_map<std::string, std::string> const& getPaths() const { return _paths; }
    std::unordered_map<std::string, Texture> const& getTextures() const { return _textures; }
    std::string const& getTerrainTexture() { return _terrain_name; }
    std::string const& getWaterTexture() { return _water_name; }
    std::function<void(std::latch& count_down)> load(Renderer * const renderer);
    std::function<void(std::latch& count_down)> loadSkybox(
      std::string_view skybox,
      Renderer* const renderer);
      void setTerrainName(std::string const& terrain_name) { _terrain_name = terrain_name; }
      void setWaterName(std::string const& water_name) { _water_name = water_name; }
      void addTexture(Texture texture) { _textures[texture.getName()] = std::move(texture); }

  private:
    void add(
      std::string const& name,
      nlohmann::json const& data,
      VkImageAspectFlags const aspect_flags,
      TEXTURE_TYPE texture_type,
      Renderer* const renderer);

  private:
    const uint32_t MAX_MIPLEVELS = 5;

    std::string _skybox_name;
    std::string _terrain_name {"_plp_terrain"};
    std::string _water_name {"_plp_water"};
    nlohmann::json _texture_config;

    std::unordered_map<std::string, std::string> _paths;
    std::unordered_map<std::string, Texture> _textures;
  };
}
