#pragma once

#include "Poulpe/Component/Mesh.hpp"

#include "Poulpe/Core/IVisitor.hpp"

#include "Poulpe/Manager/EntityManager.hpp"
#include "Poulpe/Manager/ShaderManager.hpp"
#include "Poulpe/Manager/TextureManager.hpp"

#include "Poulpe/Renderer/Adapter/VulkanAdapter.hpp"

#include <volk.h>

namespace Poulpe
{
    class Grid : public IVisitor
    {
    struct pc
    {
        glm::vec4 point;
        glm::mat4 view;
    };
    public:
        Grid(VulkanAdapter* adapter, ShaderManager* shaderManager, TextureManager* textureManager,
            VkDescriptorPool descriptorPool);

        void visit(Entity* entity) override;

    private:
        VulkanAdapter* m_Adapter;
        ShaderManager* m_ShaderManager;
        TextureManager* m_TextureManager;
        VkDescriptorPool m_DescriptorPool;

    };
}
