#pragma once

#include "Crosshair.hpp"
#include "Basic.hpp"
#include "Grid.hpp"
#include "Skybox.hpp"
#include "Normal.hpp"

namespace Poulpe
{
    class RendererFactory
    {
    public:
        template <typename T, typename... TArgs>
        static T* create(TArgs&&... args)
        {
            T* entity(new T(std::forward<TArgs>(args)...));

            return entity;
        }
    };
}
