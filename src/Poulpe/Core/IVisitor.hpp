#pragma once

#include "Poulpe/Component/Mesh.hpp"

namespace Poulpe
{
    class IVisitor
    {
    public:
        virtual void visit(Entity* entity) = 0;
        virtual ~IVisitor() = 0;
    };
}
