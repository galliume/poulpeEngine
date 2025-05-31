module;
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/fwd.hpp>

#include <memory>
#include <string>

export module Poulpe.Component.Entity;

import Poulpe.Core.PlpTypedef;
import Poulpe.Core.Logger;
import Poulpe.Utils.IDHelper;

namespace Poulpe
{
  export class Entity : public std::enable_shared_from_this<Entity>
  {
  public:
    Entity();

    IDType getID() const { return _id; }
    std::string getName() const { return _name; }

    void setName(std::string const & name) { _name = name; }
    void setVisible(bool const visible) { _is_visible = visible; }

    bool isDirty() const { return true; }
    bool isHit(glm::vec3 point);
    bool isVisible() const { return _is_visible; }

  private:
    IDType _id{ 0 };
    std::string _name{};

    bool _is_visible{ true };
  };
}
