#pragma once

#include "Poulpe/Renderer/Adapter/IRendererAdapter.hpp"
#include "Poulpe/Component/Mesh.hpp"

namespace Poulpe
{
    class IVisitor
    {
    public:
        virtual void visit(std::shared_ptr<Entity> entity) = 0;
        virtual ~IVisitor() = 0;
    };
}
