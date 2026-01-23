module;

#include <glm/glm.hpp>

export module Engine.Component.Entity;

import std;

import Engine.Core.PlpTypedef;
import Engine.Core.Logger;
import Engine.Utils.IDHelper;

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
