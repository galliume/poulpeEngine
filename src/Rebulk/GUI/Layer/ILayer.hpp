#pragma once
#include <volk.h>
#include "Rebulk/Manager/RenderManager.hpp"
#include "Rebulk/Core/CommandQueue.hpp"

namespace Rbk 
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
