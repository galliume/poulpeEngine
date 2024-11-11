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
    alignas(16) glm::mat4 projXview;
  };

  struct UniformBufferObject
  {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 projection;
    alignas(4) float constant{ 1.0f };
    alignas(4) float linear{ 0.09f };
    alignas(4) float quadratic{ 0.032f };
    alignas(8) glm::vec2 texSize;
  };

  struct CubeUniformBufferObject : UniformBufferObject
  {
    int index;
  };

  struct Light
  {
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 position;
    //ambiance diffuse specular
    alignas(16) glm::vec3 ads;
    //constant, linear, quadratiq
    alignas(16) glm::vec3 clq;
    //cutOff, outerCutoff Blank
    alignas(16) glm::vec3 coB{ 1.0f, 0.0f, 0.0f };
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
    alignas(16) glm::mat4 lightSpaceMatrix;
  };

  struct Material
  {
    alignas(16) glm::vec3 ambient { 1 };
    alignas(16) glm::vec3 diffuse { 1 };
    alignas(16) glm::vec3 specular { 1 };
    alignas(16) glm::vec3 transmittance { 1 };
    alignas(16) glm::vec3 emission { 1 };
    //shininess, ior, diss
    alignas(16) glm::vec3 shiIorDiss { 1 };
  };

  struct ObjectBuffer
  {
    Light ambientLight;
    std::array<Light, 2> pointLights;
    Light spotLight;
    Material material;
  };

  struct material_t {
    std::string name;
    glm::vec3 ambient{ 1.0 };
    glm::vec3 diffuse{ 1.0 };
    glm::vec3 specular{ 1.0 };
    glm::vec3 transmittance{ 1.0 };
    glm::vec3 emission{ 1.0 };
    float shininess{ 1.0 };
    float ior{ 1.0 };       // index of refraction
    float dissolve{ 1.0 };  // 1 == opaque; 0 == fully transparent
    int illum{ 1 };
    std::string ambientTexname;             // map_Ka
    std::string diffuseTexname;             // map_Kd
    std::string specularTexname;            // map_Ks
    std::string specularHighlightTexname;  // map_Ns
    std::string bumpTexname;                // map_bump, map_Bump, bump
    std::string alphaTexname; // map_d
  };

  struct constants
  {
    //textureID blank blank
    alignas(16) glm::vec3 textureIDBB;
    alignas(16) glm::mat4 view;
    alignas(16) glm::vec4 viewPos;
    alignas(16) glm::vec4 totalPosition{ 1.0 };
  };

  struct shadowMapConstants
  {
    alignas(16) glm::mat4 lightSpaceMatrix;
    alignas(16) glm::vec3 pos;
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
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _Indices;
    std::vector<UniformBufferObject> _ubos;
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

  //struct BBox
  //{
  //  glm::mat4 position;
  //  glm::vec3 center;
  //  glm::vec3 size;
  //  std::unique_ptr<Mesh> mesh;
  //  float maxX;
  //  float minX;
  //  float maxY;
  //  float minY;
  //  float maxZ;
  //  float minZ;
  //};
  struct EntityOptions
  {
    std::string_view shader{};
    glm::vec3 const& pos{};
    glm::vec3 const& scale{};
    glm::vec3 rotation{};
    bool hasBbox{ false };
    bool hasAnimation{ false };
    bool isPointLight{ false };
    std::vector<std::string> animationScripts{};
    bool hasShadow{ false };
    bool shouldInverseTextureY{ false };
    bool isIndexed{ false };
  };

  struct Bone {
    unsigned int id{};
    std::string name{};
    glm::mat4 offsetMatrix{};
  };

  struct Animation {
    unsigned int id;
    std::string name{};
    float duration{ 0.0 };
  };

  struct Rotation {
    unsigned int animationId;
    double time;
    glm::quat value;
  };

  struct Position {
    unsigned int animationId;
    double time;
    glm::vec3 value;
  };

  struct Scale {
    unsigned int animationId;
    double time;
    glm::vec3 value;
  };

  struct PlpMeshData
  {
    std::string name{};
    unsigned int id{};
    uint32_t materialId{ 0 };
    std::vector<uint32_t> facesMaterialId{};
    std::vector<uint32_t> indices{};
    std::vector<uint32_t> materialsID{};
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
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descPool;
    VkDescriptorSetLayout descSetLayout;
    VkDescriptorSet descSet;
    VkPipelineCache pipelineCache;
    VkPipeline pipeline;
    std::vector<VkPipelineShaderStageCreateInfo> shaders{};
  };
}
