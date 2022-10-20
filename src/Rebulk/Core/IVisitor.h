#pragma once

#include <volk.h>

#include "Rebulk/Renderer/Adapter/IRendererAdapter.h"
#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    class IVisitor
    {
    public:
        virtual void Visit(std::shared_ptr<Entity> entity) = 0;
        virtual ~IVisitor() = 0;
    };
}
