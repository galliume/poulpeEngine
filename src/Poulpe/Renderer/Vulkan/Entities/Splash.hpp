#pragma once

#include "Poulpe/Core/IVisitor.hpp"

#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/ShaderManager.hpp"
#include "Poulpe/Manager/SpriteAnimationManager.hpp"
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
        Splash(VulkanAdapter* adapter, EntityManager* entityManager,
            ShaderManager* shaderManager, TextureManager* textureManager,
            SpriteAnimationManager* spriteAnimationManager, VkDescriptorPool descriptorPool);

        void visit(Entity* entity) override;

    private:
        VulkanAdapter* m_Adapter;
        EntityManager* m_EntityManager;
        ShaderManager* m_ShaderManager;
        TextureManager* m_TextureManager;
        SpriteAnimationManager* m_SpriteAnimationManager;
        VkDescriptorPool m_DescriptorPool;
    };
}
