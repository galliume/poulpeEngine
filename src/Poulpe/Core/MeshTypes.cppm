module;
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>
#include <vector>
#include <volk.h>

export module Poulpe.Core.MeshTypes;

import Poulpe.Renderer.Vulkan.DeviceMemory;

import Poulpe.Component.Vertex;
import Poulpe.Core.PlpTypedef;

namespace Poulpe {
  export struct Buffer {
    VkBuffer buffer{ VK_NULL_HANDLE };
    DeviceMemory* memory;
    uint32_t offset;
    unsigned long long size;
    unsigned int index{ 0 };
  };

    //@todo needs huge refactoring
  export struct Data {
    std::string _name;
    std::string _texture_prefix;
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
    std::string _root_bone_name{};
  };
}