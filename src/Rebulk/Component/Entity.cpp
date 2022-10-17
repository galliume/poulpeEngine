#include "rebulkpch.h"
#include "Entity.h"

namespace Rbk
{
    void Entity::Draw()
    {
        Rbk::Log::GetLogger()->debug("Draw nothing");
    }

    void Entity::Accept(std::shared_ptr<IVisitor> visitor)
    {
        visitor->Visit(shared_from_this());
    }

    Entity::~Entity()
    {

    }

    uint32_t const Entity::GetNextSpriteIndex()
    {
        uint32_t nextIndex = 0;

        if (m_SpritesIndex > 0 && m_SpritesIndex < m_SpritesCount) {
            nextIndex = m_SpritesIndex;
            m_SpritesIndex += 1;
        } else if (m_SpritesIndex == m_SpritesCount  || 0 == m_SpritesIndex) {
            m_SpritesIndex = 1;
        }

        //Rbk::Log::GetLogger()->debug("next sprites index {} of {} sprites", nextIndex, m_SpritesCount);

        return nextIndex;
    }

    bool Entity::IsHit(glm::vec3 point)
    {
        //Rbk::Log::GetLogger()->warn(
        //    "BBOX maxX:{} minX{} maxY:{} minY{} maxZ:{} minZ{}",
        //    m_BoundingBox.maxX,
        //    m_BoundingBox.minX,
        //    m_BoundingBox.maxY,
        //    m_BoundingBox.minY,
        //    m_BoundingBox.maxZ,
        //    m_BoundingBox.minZ
        //);

        //Rbk::Log::GetLogger()->warn(
        //    "point x:{} y{} z:{}", point.x, point.y, point.z);

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
