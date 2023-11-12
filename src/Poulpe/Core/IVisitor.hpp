#pragma once

namespace Poulpe
{
    class Entity;

    class IVisitor
    {
    public:
        IVisitor() = default;
        virtual ~IVisitor() = 0;

        virtual void visit(Entity* entity) = 0;
    };
}
