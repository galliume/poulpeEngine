#pragma once

#include "Component.hpp"

namespace Poulpe
{
    class RenderComponent : public Component
    {
    public:
        RenderComponent() : Component() {}

        void visit(std::chrono::duration<float> deltaTime, IVisitable* visitable)
        {
            Component::visit(deltaTime, visitable);
        }
    };
}
