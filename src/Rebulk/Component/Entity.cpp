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

        Rbk::Log::GetLogger()->debug("next sprites index {} of {} sprites", nextIndex, m_SpritesCount);

        return nextIndex;
    }
}
