#pragma once

#include "Poulpe/Core/IVisitor.hpp"
#include "Poulpe/Manager/SpriteAnimationManager.hpp"
#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/ShaderManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"
#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

namespace Poulpe
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
        void visit(std::shared_ptr<Entity> entity) override;

    private:
        std::shared_ptr<VulkanAdapter> m_Adapter;
        std::shared_ptr<EntityManager> m_EntityManager;
        std::shared_ptr<ShaderManager> m_ShaderManager;
        std::shared_ptr<TextureManager> m_TextureManager;
        std::shared_ptr<SpriteAnimationManager> m_SpriteAnimationManager;
        VkDescriptorPool m_DescriptorPool;
    };
}
