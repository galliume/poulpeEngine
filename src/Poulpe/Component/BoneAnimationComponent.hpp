#pragma once

#include "Component.hpp"

namespace Poulpe
{
    class BoneAnimationComponent : public Component
    {
    public:
        BoneAnimationComponent() : Component() {}

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
