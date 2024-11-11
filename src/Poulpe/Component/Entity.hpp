#pragma once

#include "Mesh.hpp"

#include "Poulpe/Utils/IDHelper.hpp"

namespace Poulpe
{
  class Entity : public std::enable_shared_from_this<Entity>
  {

  public:
    Entity();
    ~Entity() = default;
        
    inline IDType getID() const { return _ID; }
    inline std::string getName() const { return _name; }

    void setName(std::string const & name) { _name = name; }
    void setVisible(bool visible) { _IsVisible = visible; }

    bool isDirty() const { return true; }
    bool isHit(glm::vec3 point);
    bool isVisible() { return _IsVisible; }

  private:
    IDType _ID{ 0 };
    std::string _name{};

    bool _IsVisible{ true };
  };
}
