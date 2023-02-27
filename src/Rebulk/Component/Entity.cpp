#include "Entity.hpp"
#include "Rebulk/Core/IVisitor.hpp"

namespace Rbk
{
    void Entity::Draw()
    {

    }

    void Entity::Accept(std::shared_ptr<IVisitor> visitor)
    {
        visitor->Visit(shared_from_this());
    }

    uint32_t Entity::GetNextSpriteIndex()
    {
        uint32_t nextIndex = 0;

        if (m_SpritesIndex > 0 && m_SpritesIndex < m_SpritesCount) {
            nextIndex = m_SpritesIndex;
            m_SpritesIndex += 1;
        } else if (m_SpritesIndex == m_SpritesCount  || 0 == m_SpritesIndex) {
            m_SpritesIndex = 1;
        }

        return nextIndex;
    }

    bool Entity::IsHit(glm::vec3 point)
    {
        RBK_DEBUG(
           "BBOX maxX:{} minX{} maxY:{} minY{} maxZ:{} minZ{}",
           GetBBox()->maxX,
           GetBBox()->minX,
           GetBBox()->maxY,
           GetBBox()->minY,
           GetBBox()->maxZ,
           GetBBox()->minZ
        );

        RBK_DEBUG(
           "point x:{} y{} z:{}", point.x, point.y, point.z);

    /*    return (
            point.x >= m_BoundingBox.minX &&
            point.x <= m_BoundingBox.maxX &&
            point.y >= m_BoundingBox.minY &&
            point.y <= m_BoundingBox.maxY &&
            point.z >= m_BoundingBox.minZ &&
            point.z <= m_BoundingBox.maxZ
        );*/
        return false;
    }
}
