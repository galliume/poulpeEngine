#pragma once

#include "Component.hpp"

namespace Poulpe
{
    class AnimationComponent : public Component
    {
    public:
        AnimationComponent() : Component() {}

        template<typename T>
        void init(T* componentImpl)
        {
          m_Pimpl = std::move(componentImpl);
        }

        void visit(std::chrono::duration<float> deltaTime, Mesh* mesh)
        {
            Component::visit(deltaTime, mesh);
        }
    };
}
