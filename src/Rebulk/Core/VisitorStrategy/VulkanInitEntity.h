#pragma once

#include <volk.h>
#include "Rebulk/Core/IVisitor.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"
#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    class VulkanInitEntity : public IVisitor
    {
        struct pc
        {
            uint32_t textureID;
            glm::vec3 cameraPos;
            float ambiantLight;
            float fogDensity;
            glm::vec3 fogColor;
            glm::vec3 lightPos;
        };

    public:
        VulkanInitEntity(std::shared_ptr<VulkanAdapter> adapter, VkDescriptorPool descriptorPool);
        void Visit(std::shared_ptr<Entity> entity) override;

    private:
        void CreateBBoxEntity(Rbk::Entity::BBox box);

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        VkDescriptorPool m_DescriptorPool;
    };
}
