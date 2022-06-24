#include "rebulkpch.h"
#include "VulkanEntityDraw.h"

namespace Rbk
{
    struct constants;

    VulkanEntityDraw::VulkanEntityDraw(
        std::shared_ptr<VulkanAdapter> adapter,
        VkDescriptorPool descriptorPool) :
        m_Adapter(adapter),
        m_DescriptorPool(descriptorPool)
    {

    }

    void VulkanEntityDraw::Visit(std::shared_ptr<Entity> entity)
    {
        std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(entity);
        if (!mesh) return;

        
    }
}