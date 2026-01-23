module;

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

export module Engine.Core.BoneTypes;

import std;

namespace Poulpe
{
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
    bool is_weightless { false };
  };
}
