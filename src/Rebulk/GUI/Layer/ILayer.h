#pragma once
#include "Rebulk/Manager/IRenderManager.h"
#include "Rebulk/GUI/ImGui/Im.h"
#include "ILayer.h"
#include "Rebulk/GUI/Window.h"
#include "Rebulk/Application.h"

namespace Rbk 
{
    class ILayer
    {
    public:
        virtual void Init() = 0;
        virtual void AddRenderManager(std::shared_ptr<IRenderManager> renderManager) = 0;

        //@todo temp, should create a struct not specific to vulkan
        virtual void Render(double timeStep, VkPhysicalDeviceProperties devicesProps) = 0;
    };
}
