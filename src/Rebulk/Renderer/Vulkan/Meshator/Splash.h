#pragma once

#include "Rebulk/Core/IVisitor.h"
#include "Rebulk/Manager/SpriteAnimationManager.h"
#include "Rebulk/Manager/EntityManager.h"
#include "Rebulk/Manager/ShaderManager.h"
#include "Rebulk/Manager/TextureManager.h"
#include "Rebulk/Renderer/Adapter/VulkanAdapter.h"

namespace Rbk
{
    class Splash : public IVisitor
    {
    struct pc
    {
        uint32_t textureID;
    };

    public:
        Splash(
            std::shared_ptr<VulkanAdapter> adapter,
            std::shared_ptr<EntityManager> entityManager,
            std::shared_ptr<ShaderManager> shaderManager,
            std::shared_ptr<TextureManager> textureManager,
            std::shared_ptr<SpriteAnimationManager> spriteAnimationManager,
            VkDescriptorPool descriptorPool
        );
        void Visit(std::shared_ptr<Entity> entity) override;

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<EntityManager> m_EntityManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
        std::shared_ptr<TextureManager> m_TextureManager;
        std::shared_ptr<SpriteAnimationManager> m_SpriteAnimationManager;
        VkDescriptorPool m_DescriptorPool;
    };
}
