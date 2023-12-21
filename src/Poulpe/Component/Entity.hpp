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
        
        IDType getID() const { return m_ID; }
        [[nodiscard]] inline const std::string getName() const { return m_Name; }

        void setName(std::string const & name) { m_Name = name; }
        void setVisible(bool visible) { m_IsVisible = visible; }

        bool isDirty() { return true; }
        bool isHit(glm::vec3 point);
        bool isVisible() { return m_IsVisible; }

    private:
        IDType m_ID;
        std::string m_Name;

        bool m_IsVisible{ true };
    };
}
