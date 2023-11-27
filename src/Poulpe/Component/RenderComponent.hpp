#pragma once

#include "Component.hpp"

namespace Poulpe
{
    class RenderComponent : public Component, public IVisitor
    {
    public:
        RenderComponent();

        //@todo mmh ? CRTP ?
        void visit(float const deltaTime, Mesh* mesh) override {
            Component::visit(deltaTime, mesh);
        }
    };
}