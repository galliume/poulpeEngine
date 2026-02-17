export module Engine.Renderer.RendererComponentTypes;

import std;

import Engine.Component.Mesh;
import Engine.Component.Texture;

import Engine.Core.Camera;
import Engine.Core.GLM;
import Engine.Core.FontTypes;
import Engine.Core.FreeType;
import Engine.Core.LightTypes;
import Engine.Core.MeshTypes;
import Engine.Core.PlpTypedef;
import Engine.Core.Volk;

namespace Poulpe {
  export struct RendererContext
  {
    enum class Mode {
      CREATION,
      UPDATE
    };

    // Environment & Animation
    Camera* camera{ nullptr };
    glm::mat4 camera_view{ 1.0f };
    double elapsed_time{ 0.0 };
    std::uint32_t env_options{ 0 };

    // Lighting
    Light sun_light{};
    std::span<Light> point_lights{};
    std::span<Light> spot_lights{};
    Buffer light_buffer{};

    // Textures & Resources
    std::unordered_map<std::string, Texture> const* textures{ nullptr };
    std::string_view skybox_name{};
    std::string_view terrain_name{};
    std::string_view water_name{};

    // UI & Fonts
    std::span<FontCharacter> characters{};
    FT_Face face{};
    std::uint32_t atlas_width{ 0 };
    std::uint32_t atlas_height{ 0 };

    Mode mode{ Mode::CREATION };

    // Pass State
    VkShaderStageFlags stage_flag_bits { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };
    bool normal_debug{ false };
    bool has_alpha_blend{ false };
  };

  /** env options config :
    HAS_FOG << 0
  **/
}
