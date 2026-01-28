export module Engine.Core.PlpTypedef;

import std;

import Engine.Core.BoneTypes;
import Engine.Core.MaterialTypes;
import Engine.Core.GLM;
import Engine.Core.Vertex;


namespace Poulpe
{
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

  export struct alignas(16) ObjectBuffer
  {
    Material material;
    std::vector<glm::mat4>bone_matrices;
  };

  export struct constants
  {
    alignas(16) glm::mat4 view;
    alignas(16) glm::vec4 view_position;
    alignas(16) std::uint32_t env_options{ 0 };
    alignas(16) std::uint32_t options{ 0 };
  };

  export struct shadowMapConstants
  {
    alignas(16) glm::mat4 view;
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
    std::uint32_t default_anim{ 0 };
  };

  export struct PlpMeshData
  {
    std::string name{};
    std::string texture_prefix{};
    std::uint64_t id{};
    std::uint32_t material_ID{ 0 };
    std::vector<std::uint32_t> face_material_ID{};
    std::vector<std::uint32_t> indices{};
    std::vector<Vertex> vertices{};
    std::vector<VertexBones> vertices_bones{};
    glm::mat4 transform_matrix{};
    glm::mat4 local_transform{};
    glm::mat4 inverse_transform_matrix{};
    std::unordered_map<std::string, Bone> bones{};
    std::string root_bone_name{};
    std::uint32_t _default_anim{};
    glm::vec3 bbox_min{};
    glm::vec3 bbox_max{};
  };
}
