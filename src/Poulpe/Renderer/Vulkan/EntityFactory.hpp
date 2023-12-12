#pragma once

#include "Poulpe/Renderer/Vulkan/Entities/Crosshair.hpp"
#include "Poulpe/Renderer/Vulkan/Entities/Basic.hpp"
#include "Poulpe/Renderer/Vulkan/Entities/Grid.hpp"
#include "Poulpe/Renderer/Vulkan/Entities/Skybox.hpp"

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