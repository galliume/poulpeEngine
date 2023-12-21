#pragma once

#include "Component.hpp"
#include "Mesh.hpp"

namespace Poulpe
{
    class MeshComponent : public Component
    {
    public:
        MeshComponent() : Component() {};

        //@todo mmh ? CRTP ?
        void visit(float const deltaTime, IVisitable* visitable)
        {
            Component::visit(deltaTime, visitable);
        }
    };
}