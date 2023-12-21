#pragma once

#include "Poulpe/Component/Renderer/Crosshair.hpp"
#include "Poulpe/Component/Renderer/Basic.hpp"
#include "Poulpe/Component/Renderer/Grid.hpp"
#include "Poulpe/Component/Renderer/Skybox.hpp"

namespace Poulpe
{
    class EntityFactory
    {
    public:
        template <typename T, typename... TArgs>
        static T& create(TArgs&&... args)
        {
            T* entity(new T(std::forward<TArgs>(args)...));

            return *entity;
        }
    };
}