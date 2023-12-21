#pragma once

#include "Component.hpp"

namespace Poulpe
{
    class AnimationComponent : public Component
    {
    public:
        AnimationComponent() : Component() {};

        template<typename T>
        void init(T* componentImpl)
        {
          m_Pimpl = std::move(componentImpl);
        }

        void visit(float const deltaTime, Mesh* mesh)
        {
            Component::visit(deltaTime, mesh);
        }
    };
}