export module Engine.Renderer.RendererComponentTypes;

import std;

import Engine.Component.Mesh;
import Engine.Component.Texture;

import Engine.Core.FontTypes;
import Engine.Core.FreeType;
import Engine.Core.LightTypes;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Core.Volk;

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
    std::uint32_t atlas_width{0};
    std::uint32_t atlas_height{0};
    MODE mode {MODE::CREATION};
    Buffer light_buffer{};
  };
}
