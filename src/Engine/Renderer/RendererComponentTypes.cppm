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
    std::unordered_map<std::string, Texture> const * textures{nullptr};
    std::string skybox_name{};
    std::string terrain_name{};
    std::string water_name{};
    Light sun_light{};
    std::span<Light, std::dynamic_extent> point_lights{};
    std::span<Light, std::dynamic_extent> spot_lights{};
    std::span<FontCharacter, std::dynamic_extent> characters{};
    FT_Face face{};
    uint32_t atlas_width{0};
    uint32_t atlas_height{0};
    MODE mode {MODE::CREATION};
    Buffer light_buffer{};
  };
}
