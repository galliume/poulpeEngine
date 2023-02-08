#pragma once
#include <volk.h>
#include "Rebulk/Manager/RenderManager.hpp"

namespace Rbk 
{
    class ILayer
    {
    public:
        ILayer() = default;
        ~ILayer() = default;

        virtual void Init() = 0;
        virtual void AddRenderManager(std::shared_ptr<RenderManager> renderManager) = 0;

        //@todo temp, should create a struct not specific to vulkan
        virtual void Render(double timeStep, VkPhysicalDeviceProperties devicesProps) = 0;
    };
}
