#pragma once

#include "Mesh.hpp"

#include "Poulpe/Core/IVisitor.hpp"

#include "Poulpe/Utils/IDHelper.hpp"

namespace Poulpe
{
  class Entity : public std::enable_shared_from_this<Entity>
  {

  public:
    Entity();
    ~Entity() = default;
        
    inline IDType getID() const { return m_ID; }
    inline std::string getName() const { return m_Name; }

    void setName(std::string const & name) { m_Name = name; }
    void setVisible(bool visible) { m_IsVisible = visible; }

    bool isDirty() const { return true; }
    bool isHit(glm::vec3 point);
    bool isVisible() { return m_IsVisible; }

  private:
    IDType m_ID{ 0 };
    std::string m_Name{};

    bool m_IsVisible{ true };
  };
}
