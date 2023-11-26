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
        Splash(VulkanAdapter* adapter, ShaderManager* shaderManager, TextureManager* textureManager,
            SpriteAnimationManager* spriteAnimationManager, VkDescriptorPool descriptorPool);

        void visit(Mesh* mesh);

    private:
        VulkanAdapter* m_Adapter;
        ShaderManager* m_ShaderManager;
        TextureManager* m_TextureManager;
        SpriteAnimationManager* m_SpriteAnimationManager;
        VkDescriptorPool m_DescriptorPool;
    };
}
