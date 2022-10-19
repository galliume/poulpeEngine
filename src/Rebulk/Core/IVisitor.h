#pragma once

#include <memory>
#include <map>
#include <any>
#include <string>

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
