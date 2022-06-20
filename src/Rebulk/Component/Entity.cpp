#include "rebulkpch.h"
#include "Entity.h"

namespace Rbk
{
    void Entity::Draw()
    {
        Rbk::Log::GetLogger()->debug("Draw nothing");
    }

    void Entity::Prepare(std::shared_ptr<IVisitor> visitor)
    {
        visitor->VisitPrepare(this);
    }

    Entity::~Entity()
    {

    }
}
