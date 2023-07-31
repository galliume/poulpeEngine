#pragma once
#include <volk.h>
#include "Poulpe/Manager/RenderManager.hpp"
#include "Poulpe/Core/CommandQueue.hpp"

namespace Poulpe 
{
    class ILayer
    {
    public:
        ILayer() = default;
        ~ILayer() = default;

        virtual void Init(Window* window, std::shared_ptr<CommandQueue> cmdQueue) = 0;
        virtual void AddRenderManager(RenderManager* renderManager) = 0;
        virtual void Render(double timeStep) = 0;
    };
}
