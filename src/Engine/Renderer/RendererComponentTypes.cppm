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

export module Engine.Renderer.RendererComponentTypes;

import Engine.Component.Texture;
import Engine.Core.PlpTypedef;
import Engine.Renderer.Mesh;

namespace Poulpe {
  export struct ComponentRenderingInfo
  {
    //@todo find a better name
    enum class MODE {
      CREATION,
      UPDATE
    };

    Mesh* mesh;
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
    MODE mode {MODE::CREATION};
  };
}
