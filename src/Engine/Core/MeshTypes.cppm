module;

#include <glm/glm.hpp>

#include <volk.h>

export module Engine.Core.MeshTypes;

import std;

import Engine.Core.Vertex;
import Engine.Core.PlpTypedef;

namespace Poulpe {
  export struct Buffer {
    VkBuffer buffer{ VK_NULL_HANDLE };
    void* memory; //@todo mmmh
    std::uint64_t offset;
    std::uint64_t size;
    uint32_t index{ 0 };
  };

    //@todo needs huge refactoring
  export struct Data {
    std::size_t _id;
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
    std::vector<VertexBones> _vertices_bones;
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
    glm::mat4 _local_transform;
    glm::mat4 _inverse_transform_matrix;
    std::unordered_map<std::string, Bone> _bones{};
    std::string _root_bone_name{};
    std::uint32_t _default_anim{};
    glm::vec3 _bbox_min{};
    glm::vec3 _bbox_max{};
    bool _is_dirty {false};
    std::uint32_t _material_id {0};
  };
}
