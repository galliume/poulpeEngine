#pragma once

#include "Poulpe/Manager/RenderManager.hpp"

#include <volk.h>

namespace Poulpe
{
    class ILayer
    {
    public:
        ILayer() = default;
        ~ILayer() = default;

        virtual void init(Window* window) = 0;
        virtual void addRenderManager(RenderManager* renderManager) = 0;
        virtual void render(float timeStep) = 0;
    };
}
