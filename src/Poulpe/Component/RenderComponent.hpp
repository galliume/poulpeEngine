#pragma once

#include "Component.hpp"

namespace Poulpe
{
    class RenderComponent : public Component
    {
    public:
        RenderComponent() : Component() {};

        //@todo mmh ? CRTP ?
        void visit(float const deltaTime, Mesh* mesh)
        {
            Component::visit(deltaTime, mesh);
        }
    };
}