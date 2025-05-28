module;
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ttnameid.h>
#include <volk.h>

#include <concepts>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

export module Poulpe.Renderer.RendererComponentTypes;

import Poulpe.Component.Texture;
import Poulpe.Core.PlpTypedef;
import Poulpe.Renderer.Mesh;

namespace Poulpe {
  export struct ComponentRenderingInfo
  {
    Mesh* const mesh;
    std::unordered_map<std::string, Texture>const& textures;
    std::string const& skybox_name;
    std::string const& terrain_name;
    std::string const& water_name;
    Light const& sun_light;
    std::vector<Light> const& point_lights;
    std::vector<Light> const& spot_lights;
    std::vector<FontCharacter> const& characters;
    FT_Face const& face;
    uint32_t const atlas_width{0};
    uint32_t const atlas_height{0};
  };
}
