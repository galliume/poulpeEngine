#pragma once

#include "Component.hpp"

namespace Poulpe
{
    class RenderComponent : public Component
    {
    public:
        RenderComponent() : Component() {};

        void visit(float const deltaTime, IVisitable* visitable)
        {
            Component::visit(deltaTime, visitable);
        }
    };
}