#pragma once

#include "IMesh.hpp"

#include "Poulpe/Core/IVisitor.hpp"

#include "Poulpe/Manager/SpriteAnimationManager.hpp"

namespace Poulpe
{
    class Splash : public IVisitor
    {
    struct pc
    {
        uint32_t textureID;
    };

    public:
        Splash(VulkanAdapter* adapter, ShaderManager* shaderManager, TextureManager* textureManager,
            SpriteAnimationManager* spriteAnimationManager, VkDescriptorPool descriptorPool);

        void visit(float const deltaTime, Mesh* mesh) override;

    private:
        VulkanAdapter* m_Adapter;
        ShaderManager* m_ShaderManager;
        TextureManager* m_TextureManager;
        SpriteAnimationManager* m_SpriteAnimationManager;
        VkDescriptorPool m_DescriptorPool;
    };
}
