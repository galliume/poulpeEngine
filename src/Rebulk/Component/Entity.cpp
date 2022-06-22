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
}
