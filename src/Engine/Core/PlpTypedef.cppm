module;

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <vulkan/vulkan.h>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>

export module Engine.Core.PlpTypedef;

import Engine.Component.Vertex;

namespace Poulpe
{
  //default textures const, needs a real assets management (unique id etc.)
  export inline constexpr std::string PLP_EMPTY{ "_plp_empty" };
  export inline constexpr std::string PLP_SAND{ "sand" };
  export inline constexpr std::string PLP_GRASS{ "grass" };
  export inline constexpr std::string PLP_GROUND{ "ground" };
  export inline constexpr std::string PLP_LOW_NOISE{ "low_noise" };
  export inline constexpr std::string PLP_HI_NOISE { "hi_noise" };
  export inline constexpr std::string PLP_SNOW{ "snow" };
  export inline constexpr std::string PLP_WATER_NORMAL_1{ "_water_normal" };
  export inline constexpr std::string PLP_WATER_NORMAL_2{ "_water_normal2" };

  export struct CameraUBO {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 proj_x_view;
  };

  export struct UniformBufferObject
  {
    alignas(16) glm::mat4 model;
    //alignas(16) glm::mat4 inversed_model;
    alignas(16) glm::mat4 projection;
  };

  export struct CubeUniformBufferObject : UniformBufferObject
  {
    int index;
  };

  export struct Light
  {
    alignas(16) glm::mat4 light_space_matrix;
    alignas(16) glm::mat4 projection;
    alignas(16) glm::mat4 view;
    //ambiance diffuse specular
    alignas(16) glm::vec3 ads;
    alignas(16) glm::vec3 clq;
    //constant, linear, quadratiq
    alignas(16) glm::vec3 coB{ 1.0f, 0.0f, 0.0f };
    //cutOff, outerCutoff Blank
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 position;
  };

  export struct Material
  {
    alignas(16) glm::vec4 base_color { 1 };
    alignas(16) glm::vec4 ambient { 1 };
    alignas(16) glm::vec4 diffuse { 1 };
    alignas(16) glm::vec4 specular { 1 };
    alignas(16) glm::vec3 transmittance { 1 };
    alignas(16) glm::vec3 shi_ior_diss { 0 }; //shininess, ior, diss
    alignas(16) glm::vec3 alpha { 0 };//{x:alpha mode, y: cutoff);
    alignas(16) glm::vec3 mre_factor{0.0, 0.0, 1.0};//x:metallic, y:roughness, z:emissive
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
  };

  export struct ObjectBuffer
  {
    Light sun_light;
    std::array<Light, 2> point_lights;
    Light spot_light;
    Material material;
    std::vector<glm::mat4>bone_matrices;
  };

  export enum class TextureWrapMode {
    WRAP,
    CLAMP_TO_EDGE,
    MIRROR_REPEAT
  };

  export struct material_t {
    std::string name;
    glm::vec4 base_color{1.0};
    glm::vec4 ambient{ 1.0 };
    glm::vec4 diffuse{ 1.0 };
    glm::vec4 specular{ 1.0 };
    glm::vec3 transmittance{ 1.0 };
    glm::vec3 mre_factor{ 0.0, 0.0, 1.0 };//x:metallic, y:roughness, z: blank
    float shininess{ 0.0 };
    float ior{ 0.0 };       // index of refraction
    float dissolve{ 1.0 };  // 1 == opaque; 0 == fully transparent
    int illum{ 1 };
    bool double_sided{ false };
    float alpha_mode{ 0.0 }; //0.0: OPAQUE 1.0: MASK 2.0: BLEND
    float alpha_cut_off{ 1.0 };

    std::string name_texture_ambient;             // map_Ka
    TextureWrapMode texture_ambient_wrap_mode_u;
    TextureWrapMode texture_ambient_wrap_mode_v;

    std::string name_texture_diffuse;             // map_Kd
    TextureWrapMode texture_diffuse_wrap_mode_u;
    TextureWrapMode texture_diffuse_wrap_mode_v;

    std::string name_texture_specular;            // map_Ks
    TextureWrapMode texture_specular_wrap_mode_u;
    TextureWrapMode texture_specular_wrap_mode_v;

    std::string name_texture_specular_highlight;  // map_Ns
    TextureWrapMode texture_specular_highlight_wrap_mode_u;
    TextureWrapMode texture_specular_highlight_wrap_mode_v;

    std::string name_texture_bump;                // map_bump, map_Bump, bump
    TextureWrapMode texture_bump_wrap_mode_u;
    TextureWrapMode texture_bump_wrap_mode_v;

    std::string name_texture_alpha; // map_d
    TextureWrapMode texture_alpha_wrap_mode_u;
    TextureWrapMode texture_alpha_wrap_mode_v;

    std::string name_texture_metal_roughness; //metal roughness
    TextureWrapMode texture_metal_roughness_wrap_mode_u;
    TextureWrapMode texture_metal_roughness_wrap_mode_v;

    std::string name_texture_emissive;
    TextureWrapMode texture_emissive_wrap_mode_u;
    TextureWrapMode texture_emissive_wrap_mode_v;

    std::string name_texture_ao;
    TextureWrapMode texture_ao_wrap_mode_u;
    TextureWrapMode texture_ao_wrap_mode_v;

    std::string name_texture_base_color;
    TextureWrapMode texture_base_color_wrap_mode_u;
    TextureWrapMode texture_base_color_wrap_mode_v;

    std::string name_texture_transmission;
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
  };

  export struct constants
  {
    alignas(16) glm::mat4 view;
    alignas(16) glm::vec3 view_position;
    alignas(16) glm::vec4 options{ 0.0 };
  };

  export struct shadowMapConstants
  {
    alignas(16) glm::mat4 view;
  };

  export struct BoneWeight {
    uint32_t vertex_id;
    float weight;
  };

  export struct Bone {
    uint32_t id;
    std::string name{};
    std::string parent_name{};
    glm::mat4 transform{};
    glm::mat4 t_pose{};
    glm::vec3 t_pose_position{};
    glm::vec3 t_pose_scale{};
    glm::quat t_pose_rotation;
    glm::mat4 offset_matrix{};
    std::vector<BoneWeight> weights{};
    std::vector<std::string> children{};
  };

  export struct EntityOptions
  {
    std::string_view shader{};
    glm::vec3 const& pos{};
    glm::vec3 const& scale{};
    glm::quat rotation;
    bool has_bbox{ false };
    bool has_animation{ false };
    bool is_point_light{ false };
    std::vector<std::string> animation_scripts{};
    bool has_shadow{ false };
    bool flip_Y{ false };
    bool is_indexed{ false };
    bool debug_normal{ false };
  };

  export struct Animation {
    uint32_t id;
    std::string name{};
    double duration{ 0.0 };
    double ticks_per_s{ 25.0 };
  };

  export enum class AnimInterpolation {
    STEP,
    LINEAR,
    SPHERICAL_LINEAR,
    CUBIC_SPLINE
  };

  export struct AnimOperation {
    uint32_t id;
    uint32_t animation_ID;
    double time;
    AnimInterpolation interpolation;
  };

  export struct Rotation : public AnimOperation {
    glm::dquat value;
  };

  export struct Position : public AnimOperation {
    glm::vec3 value;
  };

  export struct Scale : public AnimOperation {
    glm::vec3 value;
  };

  export struct PlpMeshData
  {
    std::string name{};
    std::string texture_prefix{};
    uint64_t id{};
    uint32_t material_ID{ 0 };
    std::vector<uint32_t> face_material_ID{};
    std::vector<uint32_t> indices{};
    std::vector<uint32_t> materials_ID{};
    std::vector<Vertex> vertices{};
    glm::mat4 transform_matrix{};
    glm::mat4 inverse_transform_matrix{};
    std::unordered_map<std::string, Bone> bones{};
    std::string root_bone_name{};
  };

  export struct VulkanShaders
  {
    std::unordered_map<std::string, std::unordered_map<std::string, VkShaderModule>> shaders;
  };

  export struct VulkanPipeline
  {
    VkPipelineLayout pipeline_layout;
    VkDescriptorPool desc_pool;
    VkDescriptorSetLayout descset_layout;
    VkDescriptorSet descset;
    VkPipelineCache pipeline_cache;
    VkPipeline pipeline;
    VkPipeline pipeline_bis;

    std::vector<VkPipelineShaderStageCreateInfo> shaders{};
  };

  export enum class SocketStatus {
    NOT_CONNECTED,
    CONNECTED
  };

  export enum class ServerStatus {
    NOT_RUNNING,
    RUNNING
  };

  export struct FontCharacter {
    uint32_t index;
    glm::fvec2 size;
    glm::fvec2 bearing;
    long advance;
    uint32_t mem_size{ 0 };
    std::vector<int8_t> buffer;
    int pitch{ 0 };
    int x_offset{ 0 };
    int y_offset{ 0 };
  };

  export struct PipeLineCreateInfo
  {
    VkPipelineLayout pipeline_layout;
    std::string_view name;
    std::vector<VkPipelineShaderStageCreateInfo> shaders_create_info;
    VkPipelineVertexInputStateCreateInfo* vertex_input_info;
    VkCullModeFlagBits cull_mode = VK_CULL_MODE_BACK_BIT;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode polygone_mode = VK_POLYGON_MODE_FILL;
  
    bool has_color_attachment{ false };
    bool has_depth_test{ false };
    bool has_depth_write{ false };
    bool has_dynamic_culling{ false };
    bool has_dynamic_depth_bias{ false };
    bool has_stencil_test{ false };
    bool is_patch_list{ false };
  };
}
