#pragma once

#include "Poulpe/Core/IVisitor.hpp"

#include "Poulpe/Core/PlpTypedef.hpp"

#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/LightManager.hpp"

#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    class IComponentRenderer
    {
    public:

        virtual ~IComponentRenderer() = default;

        virtual void createDescriptorSet(IVisitable* const mesh) = 0;
        virtual void setPushConstants(IVisitable* const mesh) = 0;
    };
}