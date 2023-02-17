#pragma once

#include <volk.h>
#include "Rebulk/Renderer/Adapter/IRendererAdapter.hpp"
#include "Rebulk/Component/Mesh.hpp"

namespace Rbk
{
    class IVisitor
    {
    public:
        virtual void Visit(std::shared_ptr<Entity> entity) = 0;
        virtual ~IVisitor() = 0;
    };
}
