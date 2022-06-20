#pragma once

#include <volk.h>
#include "Rebulk/Core/IVisitor.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"
#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    class VisitorVulkan : public IVisitor
    {
    public:
        VisitorVulkan(std::shared_ptr<VulkanAdapter> adapter, VkDescriptorPool descriptorPool);
        void VisitPrepare(Mesh* entity) override;
        void VisitDraw(Mesh* entity) override;

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        VkDescriptorPool m_DescriptorPool;
    };
}