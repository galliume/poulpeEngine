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
    alignas(16) glm::mat4 projection;
    alignas(4) float constant{ 1.0f };
    alignas(4) float linear{ 0.09f };
    alignas(4) float quadratic{ 0.032f };
    alignas(8) glm::vec2 tex_size;
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
    alignas(16) glm::vec3 ambient { 1 };
    alignas(16) glm::vec3 diffuse { 1 };
    alignas(16) glm::vec3 specular { 1 };
    alignas(16) glm::vec3 transmittance { 1 };
    alignas(16) glm::vec3 emission { 1 };
    //shininess, ior, diss
    alignas(16) glm::vec3 shi_ior_diss { 0 };
  };

  struct ObjectBuffer
  {
    Light sun_light;
    std::array<Light, 2> point_lights;
    Light spot_light;
    Material material;
  };

  struct material_t {
    std::string name;
    glm::vec3 ambient{ 1.0 };
    glm::vec3 diffuse{ 1.0 };
    glm::vec3 specular{ 1.0 };
    glm::vec3 transmittance{ 1.0 };
    glm::vec3 emission{ 1.0 };
    float shininess{ 0.0 };
    float ior{ 0.0 };       // index of refraction
    float dissolve{ 1.0 };  // 1 == opaque; 0 == fully transparent
    int illum{ 1 };
    std::string name_texture_ambient;             // map_Ka
    std::string name_texture_diffuse;             // map_Kd
    std::string name_texture_specular;            // map_Ks
    std::string name_texture_specular_highlight;  // map_Ns
    std::string name_texture_bump;                // map_bump, map_Bump, bump
    std::string name_texture_alpha; // map_d
    std::string name_texture_metal_roughness; //metal roughness
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
  };

  struct Data {
    std::string _name;
    std::vector<std::string> _textures;
    std::string _specular_map;
    std::string _bump_map;
    std::string _normal_map;
    std::string _alpha;
    std::string _metal_roughness;
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;
    std::vector<UniformBufferObject> _ubos;
    UniformBufferObject _original_ubo;
    std::vector<uint32_t> _ubos_offset;
    Buffer _vertex_buffer { nullptr, nullptr, 0, 0 };
    Buffer _indices_buffer { nullptr, nullptr, 0, 0 };
    uint32_t _texture_index { 0 };
    glm::vec3 _origin_pos;
    glm::vec3 _origin_scale;
    glm::vec3 _origin_rotation;
    glm::vec3 _current_pos;
    glm::vec3 _current_rotation;
    glm::vec3 _current_scale;
    glm::vec3 _tangeant;
  };

  struct EntityOptions
  {
    std::string_view shader{};
    glm::vec3 const& pos{};
    glm::vec3 const& scale{};
    glm::vec3 rotation{};
    bool has_bbox{ false };
    bool has_animation{ false };
    bool is_point_light{ false };
    std::vector<std::string> animation_scripts{};
    bool has_shadow{ false };
    bool inverse_texture_y{ false };
    bool is_indexed{ false };
  };

  struct Bone {
    unsigned int id{};
    std::string name{};
    glm::mat4 offset_matrix{};
  };

  struct Animation {
    unsigned int id;
    std::string name{};
    float duration{ 0.0 };
  };

  struct Rotation {
    unsigned int animation_ID;
    double time;
    glm::quat value;
  };

  struct Position {
    unsigned int animation_ID;
    double time;
    glm::vec3 value;
  };

  struct Scale {
    unsigned int animation_ID;
    double time;
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
    std::unordered_map<std::string, std::vector<VkShaderModule>> shaders;
  };

  struct VulkanPipeline
  {
    VkPipelineLayout pipeline_layout;
    VkDescriptorPool desc_pool;
    VkDescriptorSetLayout descset_layout;
    VkDescriptorSet descset;
    VkPipelineCache pipeline_cache;
    VkPipeline pipeline;
    std::vector<VkPipelineShaderStageCreateInfo> shaders{};
  };
}
