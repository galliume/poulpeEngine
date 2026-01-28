export module Engine.Core.MaterialTypes;
 
import std;

import Engine.Core.Constants;
import Engine.Core.GLM;

namespace Poulpe
{
  export struct Material
  {
    alignas(16) glm::vec4 base_color { 1 };
    alignas(16) glm::vec4 ambient { 1 };
    alignas(16) glm::vec4 diffuse { 1 };
    alignas(16) glm::vec4 specular { 1 };
    alignas(16) glm::vec3 transmittance { 1 };
    alignas(16) glm::vec3 shi_ior_diss { 0 }; //shininess, ior, diss
    alignas(16) glm::vec3 alpha { 0 };//{x:alpha mode, y: cutoff);
    alignas(16) glm::vec3 mre_factor{0.0, 1.0, 1.0};//x:metallic, y:roughness, z:emissive
    alignas(16) glm::vec3 normal_translation{ 0.0 };
    alignas(16) glm::vec3 normal_scale{ 1.0 };
    alignas(16) glm::vec3 normal_rotation{ 0.0 };
    alignas(16) glm::vec3 ambient_translation{ 0.0 };
    alignas(16) glm::vec3 ambient_scale{ 1.0 };
    alignas(16) glm::vec3 ambient_rotation{ 0.0 };
    alignas(16) glm::vec3 diffuse_translation{ 0.0 };
    alignas(16) glm::vec3 diffuse_scale{ 1.0 };
    alignas(16) glm::vec3 diffuse_rotation{ 0.0 };
    alignas(16) glm::vec3 emissive_translation{ 0.0 };
    alignas(16) glm::vec3 emissive_scale{ 1.0 };
    alignas(16) glm::vec3 emissive_rotation{ 0.0 };
    alignas(16) glm::vec3 mr_translation{ 0.0 };
    alignas(16) glm::vec3 mr_scale{ 1.0 };
    alignas(16) glm::vec3 mr_rotation{ 0.0 };
    alignas(16) glm::vec3 transmission_translation{ 0.0 };
    alignas(16) glm::vec3 transmission_scale{ 1.0 };
    alignas(16) glm::vec3 transmission_rotation{ 0.0 };
    alignas(16) glm::vec3 strength{ 1.0 };//x: normal strength, y occlusion strength
    alignas(16) glm::vec4 emissive_color{0.0};
  };

  export struct material_t {
    std::string name;
    glm::vec4 base_color{1.0};
    glm::vec4 ambient{ 1.0 };
    glm::vec4 diffuse{ 1.0 };
    glm::vec4 specular{ 1.0 };
    glm::vec3 transmittance{ 1.0 };
    glm::vec3 mre_factor{ 0.0, 1.0, 1.0 };//x:metallic, y:roughness, z: blank
    float shininess{ 0.0 };
    float ior{ 0.0 };       // index of refraction
    float dissolve{ 1.0 };  // 1 == opaque; 0 == fully transparent
    int illum{ 1 };
    bool double_sided{ false };
    float alpha_mode{ 0.0 }; //0.0: OPAQUE 1.0: MASK 2.0: BLEND
    float alpha_cut_off{ 1.0 };

    std::string name_texture_ambient;             // map_Ka
    std::string name_texture_ambient_path;
    TextureWrapMode texture_ambient_wrap_mode_u;
    TextureWrapMode texture_ambient_wrap_mode_v;

    std::string name_texture_diffuse;             // map_Kd
    std::string name_texture_diffuse_path;
    TextureWrapMode texture_diffuse_wrap_mode_u;
    TextureWrapMode texture_diffuse_wrap_mode_v;

    std::string name_texture_specular;            // map_Ks
    std::string name_texture_specular_path;
    TextureWrapMode texture_specular_wrap_mode_u;
    TextureWrapMode texture_specular_wrap_mode_v;

    std::string name_texture_specular_highlight;  // map_Ns
    std::string name_texture_specular_highlight_path;
    TextureWrapMode texture_specular_highlight_wrap_mode_u;
    TextureWrapMode texture_specular_highlight_wrap_mode_v;

    std::string name_texture_bump;                // map_bump, map_Bump, bump
    std::string name_texture_bump_path;
    TextureWrapMode texture_bump_wrap_mode_u;
    TextureWrapMode texture_bump_wrap_mode_v;

    std::string name_texture_alpha; // map_d
    std::string name_texture_alpha_path;
    TextureWrapMode texture_alpha_wrap_mode_u;
    TextureWrapMode texture_alpha_wrap_mode_v;

    std::string name_texture_metal_roughness; //metal roughness
    std::string name_texture_metal_roughness_path;
    TextureWrapMode texture_metal_roughness_wrap_mode_u;
    TextureWrapMode texture_metal_roughness_wrap_mode_v;

    std::string name_texture_emissive;
    std::string name_texture_emissive_path;
    TextureWrapMode texture_emissive_wrap_mode_u;
    TextureWrapMode texture_emissive_wrap_mode_v;

    std::string name_texture_ao;
    std::string name_texture_ao_path;
    TextureWrapMode texture_ao_wrap_mode_u;
    TextureWrapMode texture_ao_wrap_mode_v;

    std::string name_texture_base_color;
    std::string name_texture_base_color_path;
    TextureWrapMode texture_base_color_wrap_mode_u;
    TextureWrapMode texture_base_color_wrap_mode_v;

    std::string name_texture_transmission;
    std::string name_texture_transmission_path;
    TextureWrapMode texture_transmission_wrap_mode_u;
    TextureWrapMode texture_transmission_wrap_mode_v;

    glm::vec3 normal_translation{ 0.0 };
    glm::vec3 normal_scale{ 1.0 };
    glm::vec2 normal_rotation{ 0.0 };

    glm::vec3 ambient_translation{ 0.0 };
    glm::vec3 ambient_scale{ 1.0 };
    glm::vec2 ambient_rotation{ 0.0 };

    glm::vec3 diffuse_translation{ 0.0 };
    glm::vec3 diffuse_scale{ 1.0 };
    glm::vec2 diffuse_rotation{ 0.0 };

    glm::vec3 emissive_translation{ 0.0 };
    glm::vec3 emissive_scale{ 1.0 };
    glm::vec2 emissive_rotation{ 0.0 };

    glm::vec3 mr_translation{ 0.0 };
    glm::vec3 mr_scale{ 1.0 };
    glm::vec2 mr_rotation{ 0.0 };

    glm::vec3 transmission_translation{ 0.0 };
    glm::vec3 transmission_scale{ 1.0 };
    glm::vec2 transmission_rotation{ 0.0 };

    float normal_strength{ 1.0 };
    float occlusion_strength{ 1.0 };
    float transmission_strength{ 1.0 };

    glm::vec4 emissive_color {0.0};
  };
}
