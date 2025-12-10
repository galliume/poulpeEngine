module;
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ttnameid.h>
#include <volk.h>

export module Engine.Renderer.RendererComponentTypes;

import std;

import Engine.Component.Texture;

import Engine.Core.PlpTypedef;
import Engine.Core.MeshTypes;

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
    std::unordered_map<std::string, Texture> textures{};
    std::string skybox_name{};
    std::string terrain_name{};
    std::string water_name{};
    Light sun_light{};
    std::vector<Light> point_lights{};
    std::vector<Light> spot_lights{};
    std::vector<FontCharacter> characters{};
    FT_Face face{};
    uint32_t atlas_width{0};
    uint32_t atlas_height{0};
    MODE mode {MODE::CREATION};
    Buffer light_buffer{};
  };
}
