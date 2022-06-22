#pragma once

#include <volk.h>
#include "Rebulk/Core/IVisitor.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"
#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    class VulkanHUD : public IVisitor
    {
    public:
        VulkanHUD(std::shared_ptr<VulkanAdapter> adapter, VkDescriptorPool descriptorPool);
        void Visit(std::shared_ptr<Entity> entity) override;

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        VkDescriptorPool m_DescriptorPool;
    };
}
