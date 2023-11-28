#pragma once

#include "Component.hpp"

namespace Poulpe
{
    class AnimationComponent : public Component, public IVisitor
    {
    public:
        AnimationComponent() : Component() {};

        //@todo mmh ? CRTP ?
        void visit(float const deltaTime, Mesh* mesh) override {
            Component::visit(deltaTime, mesh);
        }
    };
}