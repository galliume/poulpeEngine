#pragma once

#include "Rebulk/Core/IVisitor.h"
#include "Rebulk/Manager/EntityManager.h"
#include "Rebulk/Manager/ShaderManager.h"
#include "Rebulk/Manager/TextureManager.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"

namespace Rbk
{
    class Crosshair : public IVisitor
    {
    struct pc
    {
        uint32_t textureID;
    };

    public:
        Crosshair(
            std::shared_ptr<VulkanAdapter> adapter,
            std::shared_ptr<EntityManager> entityManager,
            std::shared_ptr<ShaderManager> shaderManager,
            std::shared_ptr<TextureManager> textureManager,
            VkDescriptorPool descriptorPool
        );
        void Visit(std::shared_ptr<Entity> entity) override;

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<EntityManager> m_EntityManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
        std::shared_ptr<TextureManager> m_TextureManager;
        VkDescriptorPool m_DescriptorPool;
    };
}
