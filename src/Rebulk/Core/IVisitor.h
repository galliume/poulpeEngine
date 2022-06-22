#pragma once
#include <memory>

namespace Rbk
{
    class Entity;

    class IVisitor
    {
    public:
        virtual void Visit(std::shared_ptr<Entity> entity) = 0;
        virtual ~IVisitor() = 0;
    };
}