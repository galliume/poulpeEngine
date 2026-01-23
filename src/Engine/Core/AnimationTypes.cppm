module;

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

export module Engine.Core.AnimationTypes;

import std;

import Engine.Core.BoneTypes;
import Engine.Core.Constants;

namespace Poulpe
{
  export struct Animation {
    std::uint32_t id;
    std::string name{};
    double duration{ 0.0 };
    double ticks_per_s{ 25.0 };
    std::uint32_t default_anim{};
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
}
