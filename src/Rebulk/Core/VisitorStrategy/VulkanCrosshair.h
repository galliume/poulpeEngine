#pragma once

#include <volk.h>
#include "Rebulk/Core/IVisitor.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"
#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    class VulkanCrosshair : public IVisitor
    {
    struct pc
    {
        uint32_t textureID;
    };

    public:
        VulkanCrosshair(std::shared_ptr<VulkanAdapter> adapter, VkDescriptorPool descriptorPool);
        void Visit(std::shared_ptr<Entity> entity) override;

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        VkDescriptorPool m_DescriptorPool;
    };
}
