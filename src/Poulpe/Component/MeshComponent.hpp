#pragma once

#include "Component.hpp"
#include "Mesh.hpp"

namespace Poulpe
{
    class MeshComponent : public Component
    {
    public:
        MeshComponent() : Component() {}

        void visit(std::chrono::duration<float> deltaTime, IVisitable* visitable)
        {
            Component::visit(deltaTime, visitable);
        }
    };
}
