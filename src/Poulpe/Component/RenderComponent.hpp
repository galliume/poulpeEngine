#pragma once

#include "Component.hpp"

namespace Poulpe
{
    class RenderComponent : public Component, public IVisitor
    {
    public:
        RenderComponent() : Component() {};

        //@todo mmh ? CRTP ?
        void visit(float const deltaTime, Mesh* mesh) override {
            Component::visit(deltaTime, mesh);
        }
    };
}