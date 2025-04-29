#pragma once

#include "Poulpe/Component/Vertex.hpp"

#include <glm/fwd.hpp>

#include <vulkan/vulkan.h>

#include <unordered_map>

namespace Poulpe
{
  class DeviceMemory;

  struct CameraUBO {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 proj_x_view;
  };

  struct UniformBufferObject
  {
    alignas(16) glm::mat4 model;
    //alignas(16) glm::mat4 inversed_model;
    alignas(16) glm::mat4 projection;
  };

  struct CubeUniformBufferObject : UniformBufferObject
  {
    int index;
  };

  struct Light
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

  struct Material
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

  struct ObjectBuffer
  {
    Light sun_light;
    std::array<Light, 2> point_lights;
    Light spot_light;
    Material material;
    std::vector<glm::mat4>bone_matrices;
  };

  enum class TextureWrapMode {
    WRAP,
    CLAMP_TO_EDGE,
    MIRROR_REPEAT
  };

  struct material_t {
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

  struct constants
  {
    alignas(16) glm::mat4 view;
    alignas(16) glm::vec3 view_position;
    alignas(16) glm::vec4 total_position{ 1.0 };
  };

  struct shadowMapConstants
  {
    alignas(16) glm::mat4 view;
  };

  struct Buffer {
    VkBuffer buffer{ VK_NULL_HANDLE };
    DeviceMemory* memory;
    uint32_t offset;
    unsigned long long size;
    unsigned int index{ 0 };
  };
 
  struct BoneWeight {
    unsigned int vertex_id;
    float weight;
  };

  struct Bone {
    unsigned int id;
    std::string name{};
    glm::mat4 offset_matrix{};
    std::vector<BoneWeight> weights{};
  };

  //@todo needs huge refactoring
  struct Data {
    std::string _name;
    std::vector<std::string> _textures;
    std::string _specular_map;
    std::string _bump_map;
    std::string _normal_map;
    std::string _alpha;
    std::string _metal_roughness;
    std::string _emissive;
    std::string _ao;
    std::string _base_color;
    std::string _transmission;
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    std::vector<std::vector<UniformBufferObject>> _ubos;
    UniformBufferObject _original_ubo;
    std::vector<uint32_t> _ubos_offset;
    Buffer _vertex_buffer { nullptr, nullptr, 0, 0 };
    Buffer _indices_buffer { nullptr, nullptr, 0, 0 };
    uint32_t _texture_index { 0 };
    glm::vec3 _origin_pos;
    glm::vec3 _origin_scale;
    glm::quat _origin_rotation;
    glm::vec3 _current_pos;
    glm::quat _current_rotation;
    glm::vec3 _current_scale;
    glm::vec3 _tangeant;
    glm::mat4 _transform_matrix;
    glm::mat4 _inverse_transform_matrix;
    std::unordered_map<std::string, Bone> _bones{};
  };

  struct EntityOptions
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

  struct Animation {
    unsigned int id;
    std::string name{};
    float duration{ 0.0 };
    float ticks_per_s{ 25.0 };
  };

  enum class AnimInterpolation {
    STEP,
    LINEAR,
    SPHERICAL_LINEAR,
    CUBIC_SPLINE
  };

  struct AnimOperation {
    unsigned int id;
    unsigned int animation_ID;
    double time;
    AnimInterpolation interpolation;
  };

  struct Rotation : public AnimOperation {
    glm::quat value;
  };

  struct Position : public AnimOperation {
    glm::vec3 value;
  };

  struct Scale : public AnimOperation {
    glm::vec3 value;
  };

  struct PlpMeshData
  {
    std::string name{};
    unsigned int id{};
    uint32_t material_ID{ 0 };
    std::vector<uint32_t> face_material_ID{};
    std::vector<uint32_t> indices{};
    std::vector<uint32_t> materials_ID{};
    std::vector<Vertex> vertices{};
    glm::mat4 transform_matrix{};
    glm::mat4 inverse_transform_matrix{};
    std::unordered_map<std::string, Bone> bones{};
  };

  enum class SocketStatus {
    NOT_CONNECTED,
    CONNECTED
  };

  enum class ServerStatus {
    NOT_RUNNING,
    RUNNING
  };

  struct VulkanShaders
  {
    std::unordered_map<std::string, std::unordered_map<std::string, VkShaderModule>> shaders;
  };

  struct VulkanPipeline
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
}
