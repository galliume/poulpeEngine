#pragma once
#include <volk.h>
#include "Poulpe/Manager/RenderManager.hpp"

namespace Poulpe 
{
    class ILayer
    {
    public:
        ILayer() = default;
        ~ILayer() = default;

        virtual void init(Window* window) = 0;
        virtual void addRenderManager(RenderManager* renderManager) = 0;
        virtual void render(double timeStep) = 0;
    };
}
