#pragma once

#include "Poulpe/Core/IVisitor.hpp"

#include "Poulpe/Core/Buffer.hpp"

#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/LightManager.hpp"

#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
{
    class IMesh
    {
    public:

        virtual ~IMesh() = default;
        virtual void setPushConstants(Mesh* mesh) = 0;
        virtual void createDescriptorSet(Mesh* mesh) = 0;
    };
}